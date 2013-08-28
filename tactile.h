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
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace tactile
{

template<typename T>
typename T::value_type mode (const T &s)
{
    std::unordered_map<typename T::value_type,size_t> dist;
    size_t m_count = 0;
    typename T::value_type m{};
    for (auto i : s)
    {
        ++dist[i];
        if (dist[i] > m_count)
        {
            m_count = dist[i];
            m = i;
        }
    }
    return m;
}

template<typename T>
float movement_variance (const T &s)
{
    if (s.size () < 2)
        return 0.0;
    const size_t total = s.size () - 1;
    float x2 = 0.0f;
    float x = 0.0f;
    typename T::value_type last = s[0];
    for (size_t i = 1; i < s.size (); ++i)
    {
        float d = last.distanceTo (s[i]);
        x2 += (d * d);
        x += d;
        last = s[i];
    }
    // var = E[x^2]-E[x]^2
    return (x2 / total) - (x / total) * (x / total);
}

template<typename T>
float movement_average (const T &s)
{
    if (s.size () < 2)
        return 0.0;
    const size_t total = s.size () - 1;
    float x = 0.0f;
    typename T::value_type last = s[0];
    for (size_t i = 1; i < s.size (); ++i)
    {
        x += last.distanceTo (s[i]);
        last = s[i];
    }
    return (x / total);
}

template<typename T>
class sliding_time_window
{
    private:
    uint64_t duration;
    std::deque<std::pair<uint64_t,T>> samples;
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
    void add_sample (uint64_t ts, const T &n)
    {
        samples.emplace_front (ts, n);
    }
    const std::vector<T> get_samples () const
    {
        std::vector<T> s (samples.size ());
        size_t index = 0;
        for (auto i : samples)
            s[index++] = i.second;
        return s;
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
    sliding_time_window<int> w;
    int current_count;
    int last_count;
    public:
    finger_counter (uint64_t duration)
        : w (duration)
        , current_count (-1)
        , last_count (-1)
    {
    }
    void update (uint64_t ts, const Leap::HandList &hands)
    {
        w.update (ts);
        if (hands.isEmpty () || !hands[0].isValid ())
            w.add_sample (ts, 0);
        else
            w.add_sample (ts, hands[0].fingers ().count ());
        if (w.full (85, ts))
        {
            last_count = current_count;
            current_count = mode (w.get_samples ());
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

class finger_pointer
{
    private:
    sliding_time_window<Leap::Vector> w;
    public:
    finger_pointer ()
        : w (800000)
    {
    }
    void update (uint64_t ts, const Leap::Finger &f)
    {
        w.update (ts);
        w.add_sample (ts, f.tipPosition ());
        if (w.full (85, ts))
        {
            std::clog << sqrt (movement_variance (w.get_samples ())) << '\t';
            std::clog << movement_average (w.get_samples ()) << std::endl;
        }
    }
};

class listener : public Leap::Listener
{
    private:
    bool done;
    frame_counter frc;
    finger_counter fic;
    finger_pointer fip;
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
        else if (fic.count () == 1)
            fip.update (f.timestamp (), f.hands ()[0].fingers ()[0]);
    }
};

}

#endif
