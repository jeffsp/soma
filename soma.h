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
    vec3 (float x, float y, float z)
        : x (x)
        , y (y)
        , z (z)
    { }
    float x;
    float y;
    float z;
};

float distance (const vec3 &a, const vec3 &b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return sqrt (dx * dx + dy * dy + dz * dz);
}

vec3 operator- (const vec3 &a, const vec3 &b)
{
    return vec3 (a.x - b.x, a.y - b.y, a.z - b.z);
}

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

const uint64_t FEATURE_WINDOW_DURATION = 500000;

struct feature_vector
{
    std::array<vec3,5> tip_velocities;
    std::array<vec3,5> tip_directions;
    std::array<float,4> between_distances;
    std::array<vec3,4> between_directions;
    feature_vector ()
    {
    }
};

bool sort_left_to_right (const Leap::Pointable &a, const Leap::Pointable &b)
{
    return a.tipPosition ().x < b.tipPosition ().x;
}

feature_vector pl_to_feature_vector (const Leap::PointableList &pl)
{
    feature_vector f;
    // only use at most 5 pointables
    size_t npoints = std::min (pl.count (), 5);
    // sort from left to right
    std::vector<Leap::Pointable> p (npoints);
    // get vector of pointables.  unfortunately, the [] operator returns a copy of the pointable, not a reference, so we
    // are forced to sort copies of the objects rather than sorting pointers to the objects.
    for (size_t i = 0; i < p.size (); ++i)
        p[i] = pl[i];
    // sort
    sort (p.begin (), p.end (), sort_left_to_right);
    // get velocities, directions
    for (size_t i = 0; i < p.size (); ++i)
    {
        f.tip_velocities[i] = p[i].tipVelocity ();
        f.tip_directions[i] = p[i].direction ();
    }
    // get between measurements
    for (size_t i = 0; i + 1 < p.size (); ++i)
    {
        f.between_distances[i] = distance (p[i].tipPosition (), p[i + 1].tipPosition ());
        f.between_directions[i] = p[i].tipPosition () - p[i + 1].tipPosition ();
    }
    return f;
}

enum class hand_position
{
    unknown,
    pointing,
    clicking,
    scrolling,
    centering
};

class hand_position_classifier
{
    private:
    sliding_time_window<feature_vector> w;
    public:
    hand_position_classifier ()
        : w (FEATURE_WINDOW_DURATION)
    {
    }
    void add_sample (const uint64_t ts, const Leap::PointableList &pl)
    {
        w.add_sample (ts, pl_to_feature_vector (pl));
    }
    hand_position classify () const
    {
        return hand_position::unknown;
    }
};

enum class hand_movement
{
    unknown,
    none,
    moving,
    clicking,
    right_clicking
};

class hand_movement_classifier
{
    private:
    sliding_time_window<feature_vector> w;
    public:
    hand_movement_classifier ()
        : w (FEATURE_WINDOW_DURATION)
    {
    }
    void add_sample (const uint64_t ts, const Leap::PointableList &pl)
    {
        w.add_sample (ts, pl_to_feature_vector (pl));
    }
    hand_movement classify () const
    {
        return hand_movement::unknown;
    }
};

}

#endif
