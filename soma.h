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

namespace soma
{

struct point
{
    float x;
    float y;
    float z;
};

typedef std::vector<point> finger_tips;

template<typename T>
std::vector<double> distances (const T &x)
{
    if (x.size () < 2)
        return std::vector<double> ();
    std::vector<double> d (x.size () - 1);
    for (size_t i = 0; i < d.size (); ++i)
        d[i] = x[i].distanceTo (x[i + 1]);
    return d;
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
    float fullness (uint64_t ts) const
    {
        float start = samples.back ().first;
        assert (start <= ts);
        return (ts - start) / duration;
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
        // don't add the same sample twice
        assert (samples.front ().first != ts);
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

const uint64_t FINGER_COUNTER_WINDOW_DURATION = 200000;
const float FINGER_COUNTER_WINDOW_FULLNESS = 0.85f;
const float FINGER_COUNTER_CERTAINTY = 0.8f;

class finger_tracker
{
    private:
    sliding_time_window<uint64_t> w;
    unsigned current_count;
    unsigned last_count;
    float current_certainty;
    public:
    finger_tracker ()
        : w (FINGER_COUNTER_WINDOW_DURATION)
        , current_count (~0)
        , last_count (~0)
        , current_certainty (0.0f)
    {
    }
        /*
    void update (uint64_t ts, const Leap::PointableList &p)
    {
        w.update (ts);
        if (p.isEmpty () || !p[0].isValid ())
            w.add_sample (ts, 0);
        else
            w.add_sample (ts, p.count ());
        // if it's mostly full
        if (w.fullness (ts) > FINGER_COUNTER_WINDOW_FULLNESS)
        {
            // get count and certainty
            auto s = w.get_samples ();
            assert (s.size () > 0);
            unsigned count = mode (s);
            float total = 0.0f;
            for (auto i : s)
                if (i == count)
                    ++total;
            float certainty = total / s.size ();
            // don't change unless certainty is high
            if (certainty < FINGER_COUNTER_CERTAINTY)
                return;
            last_count = current_count;
            current_count = count;
            current_certainty = certainty;
        }
    }
        */
    unsigned count () const
    {
        return current_count;
    }
    float certainty () const
    {
        return current_certainty;
    }
    bool is_changed () const
    {
        return current_count != last_count;
    }
};

}

#endif
