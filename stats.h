/// @file stats.h
/// @brief statistical functions
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-05

#ifndef STATS_H
#define STATS_H

#include <algorithm>
#include <cstdlib>
#include <unordered_map>

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

}

#endif
