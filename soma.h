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
#include <unistd.h>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "Leap.h"

namespace soma
{

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

bool sort_left_to_right (const Leap::Pointable &a, const Leap::Pointable &b)
{
    return a.tipPosition ().x < b.tipPosition ().x;
}

class feature_vector
{
    public:
    static const size_t size =
        5 * 3 // 5 tip velocities
        + 5 * 3 // 5 tip directions
        + 4 * 1 // 4 between distances
        + 4 * 3; // 4 between directions
    typedef std::array<float,size> container_type;
    private:
    container_type v;
    public:
    const container_type &values ()
    {
        return v;
    }
    feature_vector (const Leap::PointableList &pl)
    {
        // only use at most 5 pointables
        std::vector<Leap::Pointable> p (std::min (pl.count (), 5));
        // get vector of pointables.  unfortunately, the [] operator returns a copy of the pointable, not a reference,
        // so we are forced to sort copies of the objects rather than sorting pointers to the objects.
        for (size_t i = 0; i < p.size (); ++i)
            p[i] = pl[i];
        // sort
        sort (p.begin (), p.end (), sort_left_to_right);
        // stuff them into the vector
        size_t i = 0;
        for (size_t j = 0; j < 5; ++j)
        {
            assert (i < v.size ());
            v[i++] = j < p.size () ? p[j].tipVelocity ().x : 0.0;
            v[i++] = j < p.size () ? p[j].tipVelocity ().y : 0.0;
            v[i++] = j < p.size () ? p[j].tipVelocity ().z : 0.0;
        }
        for (size_t j = 0; j < 5; ++j)
        {
            assert (i < v.size ());
            v[i++] = j < p.size () ? p[j].direction ().x : 0.0;
            v[i++] = j < p.size () ? p[j].direction ().y : 0.0;
            v[i++] = j < p.size () ? p[j].direction ().z : 0.0;
        }
        for (size_t j = 0; j < 4; ++j)
        {
            assert (i < v.size ());
            v[i++] = j + 1 < p.size () ? p[j].tipPosition ().distanceTo (p[j + 1].tipPosition ()) : 0.0;
        }
        for (size_t j = 0; j < 4; ++j)
        {
            assert (i < v.size ());
            Leap::Vector dir = j + 1 < p.size () ? p[j].tipPosition () - p[j + 1].tipPosition () : Leap::Vector ();
            v[i++] = dir.x;
            v[i++] = dir.y;
            v[i++] = dir.z;
        }
        assert (i == v.size ());
    }
};

typedef std::vector<uint64_t> timestamps;
typedef std::vector<feature_vector> feature_vectors;

enum class hand_position
{
    unknown,
    pointing,
    clicking,
    scrolling,
    centering
};

std::string to_string (const hand_position hp)
{
    switch (hp)
    {
        default:
        throw std::runtime_error ("invalid hand position");
        case hand_position::unknown:
        return std::string ("unknown");
        case hand_position::pointing:
        return std::string ("pointing");
        case hand_position::clicking:
        return std::string ("clicking");
        case hand_position::scrolling:
        return std::string ("scrolling");
        case hand_position::centering:
        return std::string ("centering");
    }
}

template<typename T>
class stats
{
    public:
    void update (const T &x)
    {
        ++total;
        u1 += x;
        u2 += x * x;
    }
    double mean ()
    {
        return static_cast<double> (u1) / total;
    }
    double variance ()
    {
        double u = mean ();
        return static_cast<double> (u2) / total - u * u;
    }
    private:
    size_t total;
    T u1;
    T u2;
};

class hand_position_classifier
{
    private:
    std::vector<stats<float>> vs;
    public:
    hand_position_classifier ()
        : vs (feature_vector::size)
    {
    }
    void update (const hand_position hp, const feature_vectors &fvs)
    {
    }
    hand_position classify () const
    {
        return hand_position::unknown;
    }
};

enum class hand_movement
{
    unknown,
    still,
    moving,
    clicking,
    right_clicking
};

class hand_movement_classifier
{
    private:
    static const uint64_t FEATURE_WINDOW_DURATION = 500000;
    sliding_time_window<feature_vector> w;
    public:
    hand_movement_classifier ()
        : w (FEATURE_WINDOW_DURATION)
    {
    }
    void add_sample (const uint64_t ts, const Leap::PointableList &pl)
    {
        w.add_sample (ts, feature_vector (pl));
    }
    hand_movement classify () const
    {
        return hand_movement::unknown;
    }
};

}

#endif
