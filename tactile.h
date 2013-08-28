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
    uint64_t duration;
    std::deque<std::pair<uint64_t,int>> samples;
    mutable std::map<int,size_t> dist;
    public:
    sliding_time_window (uint64_t duration)
        : duration (duration)
    {
    }
    bool full (int percent, uint64_t ts) const
    {
        uint64_t start = samples.back ().first;
        assert (start <= ts);
        if (ts - start >= percent * duration / 100)
            return true;
        return false;
    }
    void update (uint64_t ts)
    {
        while (!samples.empty ())
        {
            assert (ts >= samples.back ().first);
            if (ts - samples.back ().first > duration)
                samples.pop_back ();
            else
                break;
        }
    }
    void add_sample (uint64_t ts, const int n)
    {
        samples.emplace_front (ts, n);
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
    void dump (std::ostream &s) const
    {
        uint64_t start = samples.front ().first;
        for (auto i : samples)
            s << ' ' << '<' << start - i.first << ',' << i.second << '>';
        s << std::endl;
    }
};

class frame_counter
{
    private:
    uint64_t frames;
    uint64_t first_ts;
    uint64_t last_ts;
    public:
    frame_counter ()
        : frames (0)
        , first_ts (0)
        , last_ts (0)
    {
    }
    void update (uint64_t ts)
    {
        if (frames == 0)
            first_ts = ts;
        else
            last_ts = ts;
        ++frames;
    }
    int fps () const
    {
        int secs = (last_ts - first_ts) / 1000000;
        if (secs != 0)
            return frames / secs;
        return -1;
    }
};

class finger_counter
{
    private:
    sliding_time_window stw;
    int current_count;
    int last_count;
    public:
    finger_counter (uint64_t duration)
        : stw (duration)
        , current_count (-1)
        , last_count (-1)
    {
    }
    void update (uint64_t ts, const Leap::HandList &hands)
    {
        stw.update (ts);
        if (hands.isEmpty () || !hands[0].isValid ())
            stw.add_sample (ts, 0);
        else
            stw.add_sample (ts, hands[0].fingers ().count ());
        if (stw.full (85, ts))
        {
            last_count = current_count;
            current_count = stw.mode ();
        }
    }
    int count () const
    {
        return current_count;
    }
    bool is_changed () const
    {
        return current_count != last_count;
    }
};

class listener : public Leap::Listener
{
    private:
    bool done;
    frame_counter frc;
    finger_counter fic;
    public:
    listener ()
        : done (false)
        , fic (700000)
    {
    }
    ~listener ()
    {
        std::clog << frc.fps () << "fps" << std::endl;
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
        frc.update (f.timestamp ());
        fic.update (f.timestamp (), f.hands ());
        if (fic.is_changed ())
            std::clog << " fingers " << fic.count () << std::endl;
        if (fic.count () == 5)
            done = true;
    }
};

}

#endif
