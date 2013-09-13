/// @file soma.h
/// @brief soma software for Leap controller
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-08-26

#ifndef SOMA_H
#define SOMA_H

#include <algorithm>
#include <array>
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

// Euclidean, or 3D, vector
struct vec3
{
    vec3 ()
        : x (0.0f)
        , y (0.0f)
        , z (0.0f)
    { }
    vec3 (const Leap::Vector &a)
        : x (a.x)
        , y (a.y)
        , z (a.z)
    { }
    float x;
    float y;
    float z;
};

bool operator== (const vec3 &a, const vec3 &b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool operator!= (const vec3 &a, const vec3 &b)
{
    return !(a == b);
}

/// @brief i/o helper
std::ostream& operator<< (std::ostream &s, const vec3 &p)
{
    s << p.x << ' ' << p.y << ' ' << p.z;
    return s;
}

typedef std::vector<vec3> vec3s;

/// @brief i/o helper
std::ostream& operator<< (std::ostream &s, const vec3s &p)
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
    std::deque<uint64_t> timestamps;
    std::deque<T> samples;
    void update (uint64_t ts)
    {
        while (!timestamps.empty ())
        {
            assert (samples.size () == timestamps.size ());
            assert (ts >= timestamps.back ());
            if (ts - timestamps.back () > duration)
            {
                timestamps.pop_back ();
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
    void clear ()
    {
        samples.clear ();
        timestamps.clear ();
    }
    float fullness (uint64_t ts) const
    {
        if (timestamps.empty ())
            return 0.0f;
        float start = timestamps.back ();
        assert (start <= ts);
        assert (duration != 0);
        return (ts - start) / duration;
    }
    void add_sample (uint64_t ts, const T &n)
    {
        // remove samples with old timestamps
        update (ts);
        // don't add the same sample twice
        assert (timestamps.empty () || timestamps.front () != ts);
        timestamps.emplace_front (ts);
        samples.emplace_front (n);
    }
    const std::deque<T> get_samples () const
    {
        return samples;
    }
    const std::deque<uint64_t> get_timestamps () const
    {
        return timestamps;
    }
    void dump (std::ostream &s) const
    {
        uint64_t start = samples.front ().first;
        for (auto i : samples)
            s << ' ' << '<' << start - i.first << ',' << i.second << '>';
        s << std::endl;
    }
};

vec3s get_positions (const Leap::PointableList &l)
{
    vec3s p (l.count ());
    for (size_t i = 0; i < p.size (); ++i)
        p[i] = l[i].tipPosition ();
    return p;
}

vec3s get_velocities (const Leap::PointableList &l)
{
    vec3s p (l.count ());
    for (size_t i = 0; i < p.size (); ++i)
        p[i] = l[i].tipVelocity ();
    return p;
}

vec3s get_directions (const Leap::PointableList &l)
{
    vec3s p (l.count ());
    for (size_t i = 0; i < p.size (); ++i)
        p[i] = l[i].direction ();
    return p;
}

const uint64_t FEATURE_SAMPLE_DURATION = 1000000/30;
const uint64_t FEATURE_WINDOW_DURATION = 500000;

struct feature
{
    size_t npoints;
    std::array<vec3,5> tip_positions;
    std::array<vec3,5> tip_velocities;
    std::array<vec3,5> tip_directions;
    std::array<float,4> between_distances;
    std::array<vec3,4> between_directions;
    feature ()
        : npoints (0)
    {
    }
};

bool sort_left_to_right (const Leap::Pointable &a, const Leap::Pointable &b)
{
    return a.tipPosition ().x < b.tipPosition ().x;
}

feature pointable_list_to_feature (const Leap::PointableList &pl)
{
    feature f;
    // allow a maximum of 5 pointables in a feature
    f.npoints = std::min (pl.count (), 5);
    // sort from left to right
    std::vector<Leap::Pointable> p (f.npoints);
    // get vector of pointers
    for (size_t i = 0; i < p.size (); ++i)
        p[i] = pl[i];
    // sort
    sort (p.begin (), p.end (), sort_left_to_right);
    for (size_t i = 0; i < f.npoints; ++i)
    {
        f.tip_positions[i] = pl[i].tipPosition ();
        f.tip_velocities[i] = pl[i].tipVelocity ();
        f.tip_directions[i] = pl[i].direction ();
        if (i + 1 < f.npoints)
        {
            f.between_distances[i] = distance (f.tip_positions[i], f.tip_positions[i + 1]);
            f.between_directions[i] = direction (f.tip_positions[i], f.tip_positions[i + 1]);
        }
    }
    return f;
}

class feature_window
{
    private:
    sliding_time_window<feature> w;
    public:
    feature_window ()
        : w (FEATURE_WINDOW_DURATION)
    {
    }
    void update (const unsigned count, const feature &f)
    {
    }
};

enum class hand_position
{
    unknown,
    pointing,
    clicking,
    scrolling,
    centering
};

enum class hand_movement
{
    unknown,
    none,
    up,
    down,
    left,
    right
};


}

#endif
