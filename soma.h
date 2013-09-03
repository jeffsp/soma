/// @file soma.h
/// @brief soma software for Leap controller
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-08-26

#ifndef SOMA_H
#define SOMA_H

#include "Leap.h"
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

template<typename T>
typename T::value_type mode (const T &x)
{
    std::unordered_map<typename T::value_type,size_t> dist;
    size_t m_count = 0;
    typename T::value_type m{};
    for (auto i : x)
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
double average (const T &x)
{
    if (x.empty ())
        return 0.0;
    return std::accumulate (x.begin (), x.end (), 0.0) / x.size ();
}

template<typename T>
double variance (const T &x)
{
    if (x.empty ())
        return 0.0;
    double sum2 = 0.0;
    double sum = 0.0;
    for (auto i : x)
    {
        sum2 += (i * i);
        sum += i;
    }
    // var = E[x^2]-E[x]^2
    return (sum2 / x.size ()) - (sum / x.size ()) * (sum / x.size ());
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
    sliding_time_window<uint64_t> w;
    int current_count;
    int last_count;
    public:
    finger_counter (uint64_t duration)
        : w (duration)
        , current_count (-1)
        , last_count (-1)
    {
    }
    void update (uint64_t ts, const Leap::PointableList &p)
    {
        w.update (ts);
        if (p.isEmpty () || !p[0].isValid ())
            w.add_sample (ts, 0);
        else
            w.add_sample (ts, p.count ());
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

class small_hand;
class big_hand;

template<typename H>
struct hand_traits
{
    static constexpr float pinch_min = 1.0;
    static constexpr float pinch_max = 1.0;
};

template<>
struct hand_traits<small_hand>
{
    static constexpr float pinch_min = 1.0;
    static constexpr float pinch_max = 1.0;
};

template<>
struct hand_traits<big_hand>
{
    static constexpr float pinch_min = 1.0;
    static constexpr float pinch_max = 1.0;
};

}

#endif
