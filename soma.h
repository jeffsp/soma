/// @file soma.h
/// @brief soma software for Leap controller
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-08-26

#ifndef SOMA_H
#define SOMA_H

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "Leap.h"

namespace soma
{

struct point
{
    point ()
        : x (0.0f)
        , y (0.0f)
        , z (0.0f)
    { }
    point (const Leap::Vector &a)
        : x (a.x)
        , y (a.y)
        , z (a.z)
    { }
    float x;
    float y;
    float z;
};

bool operator== (const point &a, const point &b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool operator!= (const point &a, const point &b)
{
    return !(a == b);
}

/// @brief i/o helper
std::ostream& operator<< (std::ostream &s, const point &p)
{
    s << p.x << ' ' << p.y << ' ' << p.z;
    return s;
}

typedef std::vector<point> points;

/// @brief i/o helper
std::ostream& operator<< (std::ostream &s, const points &p)
{
    for (auto i : p)
        s << ' ' << i;
    return s;
}

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
    uint64_t get_frames () const
    {
        return frames;
    }
    float fps () const
    {
        float secs = static_cast<float> (last_ts - first_ts) / 1000000;
        // don't count the last frame
        if (frames != 0 && secs != 0.0f)
            return (frames - 1) / secs;
        return -1;
    }
};

template<typename T>
class sliding_time_window
{
    private:
    const uint64_t duration;
    std::deque<uint64_t> time_stamps;
    std::deque<T> samples;
    void update (uint64_t ts)
    {
        while (!time_stamps.empty ())
        {
            assert (samples.size () == time_stamps.size ());
            assert (ts >= time_stamps.back ());
            if (ts - time_stamps.back () > duration)
            {
                time_stamps.pop_back ();
                samples.pop_back ();
            }
            else
                break;
        }
    }
    public:
    sliding_time_window (uint64_t duration)
        : duration (duration)
    {
    }
    float fullness (uint64_t ts) const
    {
        if (time_stamps.empty ())
            return 0.0f;
        float start = time_stamps.back ();
        assert (start <= ts);
        assert (duration != 0);
        return (ts - start) / duration;
    }
    void add_sample (uint64_t ts, const T &n)
    {
        // remove samples with old timestamps
        update (ts);
        // don't add the same sample twice
        assert (time_stamps.empty () || time_stamps.front () != ts);
        time_stamps.emplace_front (ts);
        samples.emplace_front (n);
    }
    const std::deque<T> get_samples () const
    {
        return samples;
    }
    void dump (std::ostream &s) const
    {
        uint64_t start = samples.front ().first;
        for (auto i : samples)
            s << ' ' << '<' << start - i.first << ',' << i.second << '>';
        s << std::endl;
    }
};

points get_points (const Leap::PointableList &l)
{
    points p (l.count ());
    for (size_t i = 0; i < p.size (); ++i)
        p[i] = l[i].tipPosition ();
    return p;
}

}

#endif
