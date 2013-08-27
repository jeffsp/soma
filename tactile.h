/// @file tactile.h
/// @brief tactile software for Leap controller
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-08-26

#ifndef TACTILE_H
#define TACTILE_H

#include "Leap.h"
#include <cassert>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace tactile
{

class sliding_time_window
{
    private:
    uint64_t duration_ms;
    std::deque<std::pair<uint64_t,int>> samples;
    mutable std::map<int,size_t> dist;
    public:
    sliding_time_window (uint64_t duration_ms)
        : duration_ms (duration_ms)
    {
    }
    void add_sample (uint64_t timestamp_ms, const int n)
    {
        samples.emplace_front (timestamp_ms, n);
        while (!samples.empty ())
        {
            assert (timestamp_ms >= samples.back ().first);
            if (timestamp_ms - samples.back ().first > duration_ms)
                samples.pop_back ();
            else
                break;
        }
    }
    int mode () const
    {
        dist.clear ();
        size_t m_count = 0;
        int m = 0;
        for (auto i : samples)
        {
            ++dist[i.second];
            if (dist[i.second] > m_count)
            {
                m_count = dist[i.second];
                m = i.second;
            }
        }
        return m;
    }
};

class listener : public Leap::Listener
{
    private:
    bool done;
    uint64_t frames;
    uint64_t first_timestamp;
    uint64_t last_timestamp;
    int last_fingers;
    sliding_time_window stw;
    public:
    listener ()
        : done (false)
        , frames (0)
        , first_timestamp (0)
        , last_timestamp (0)
        , last_fingers (0)
        , stw (800)
    {
    }
    ~listener ()
    {
        int secs = (last_timestamp - first_timestamp) / 1000000;
        if (secs != 0)
            std::clog << frames / secs << "fps" << std::endl;
        else
            std::clog << "nan fps" << std::endl;
    }
    bool is_done () const
    {
        return done;
    }
    virtual void onInit (const Leap::Controller&)
    {
        std::clog << "onInit()" << std::endl;
    }
    virtual void onConnect (const Leap::Controller&)
    {
        std::clog << "onConnect()" << std::endl;
    }
    virtual void onDisconnect (const Leap::Controller&)
    {
        std::clog << "onDisconnect()" << std::endl;
    }
    virtual void onFrame(const Leap::Controller& c)
    {
        const Leap::Frame &f = c.frame ();
        uint64_t ts = f.timestamp ();
        if (frames == 0)
            first_timestamp = ts;
        else
            last_timestamp = ts;
        ++frames;
        const Leap::HandList &hands = f.hands ();
        for (int i = 0; i < hands.count (); ++i)
        {
            const Leap::Hand &h = hands[i];
            if (h.isValid ())
            {
                stw.add_sample (ts / 1000, h.fingers ().count ());
                const int total = stw.mode ();
                if (total != last_fingers)
                {
                    std::clog << " fingers " << total << std::endl;
                    last_fingers = total;
                }
                if (total == 5)
                    done = true;
            }
        }
    }
};

}

#endif
