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
    void add_sample (uint64_t ts, const T &n)
    {
        update (ts);
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

points get_points (const Leap::PointableList &l)
{
    points p (l.count ());
    for (size_t i = 0; i < p.size (); ++i)
        p[i] = l[i].tipPosition ();
    return p;
}

}

#endif
