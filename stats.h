/// @file stats.h
/// @brief statistical functions
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-05

#ifndef STATS_H
#define STATS_H

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <map>

namespace soma
{

/// @brief get the mode of a container of numbers
///
/// @tparam T container type
/// @param x
///
/// @return the mode
template<typename T>
typename T::value_type mode (const T &x)
{
    std::map<typename T::value_type,size_t> dist;
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

/// @brief get the mean of a container of numbers
///
/// @tparam T container type
/// @param x
///
/// @return the mean
template<typename T>
double mean (const T &x)
{
    if (x.empty ())
        return 0.0;
    return std::accumulate (x.begin (), x.end (), 0.0) / x.size ();
}

/// @brief get the variance of a container of numbers
///
/// @tparam T container type
/// @param x
///
/// @return the variance
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

/// @brief keep a running sum and total that you can add and remove numbers from
class running_mean
{
    private:
    size_t total;
    double sum;
    public:
    /// @brief contructor
    running_mean ()
        : total (0)
        , sum (0)
    {
    }
    /// @brief reset to zero
    void reset ()
    {
        total = 0;
        sum = 0;
    }
    /// @brief add a number to the running sum and total
    ///
    /// @param x number to add
    void add (const double x)
    {
        ++total;
        sum += x;
    }
    /// @brief remove a number from the running sum and total
    ///
    /// @param x number to remove
    void remove (const double x)
    {
        assert (total > 0);
        assert (sum - x >= 0);
        --total;
        sum -= x;
    }
    /// @brief get the current mean
    ///
    /// @return the mean
    double mean () const
    {
        return static_cast<double> (sum) / total;
    }
};

/// @brief keep a running distribution that you can add and remove numbers from
class running_mode
{
    private:
    /// @brief the distribution
    std::map<int,size_t> d;
    /// @brief the mode
    int m;
    /// @brief count of the mode
    size_t count;
    public:
    /// @brief contructor
    running_mode ()
        : count (0)
    {
    }
    /// @brief reset to zero
    void reset ()
    {
        d.clear ();
        count = 0;
    }
    /// @brief add a number to the running distribution
    ///
    /// @param x number to add
    void add (const int x)
    {
        // update the distribution
        ++d[x];
        // if this count is greater than the mode's count, change the mode
        if (d[x] > count)
        {
            m = x;
            count = d[x];
        }
    }
    /// @brief remove a number from the running distribution
    ///
    /// @param x number to remove
    void remove (const int x)
    {
        assert (!d.empty ());
        assert (count > 0);
        assert (d[x] != 0);
        // update the count
        --d[x];
        // if this number was the mode, then the mode may have changed,
        // otherwise it could not have changed
        if (m == x)
        {
            // x is the mode, so mode's count gets decremented
            --count;
            // check all counts
            for (auto i : d)
            {
                if (i.second > count)
                {
                    m = i.first;
                    count = i.second;
                }
            }
        }
    }
    /// @brief get the current mode
    ///
    /// @return the mode
    size_t mode () const
    {
        assert (!d.empty ());
        return m;
    }
};

}

#endif
