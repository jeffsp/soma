/// @file sliding_window.h
/// @brief sliding_window class
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-23

#ifndef SLIDING_WINDOW_H
#define SLIDING_WINDOW_H

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <deque>

namespace soma
{

/// @brief dummy class for adding a sample with no observer
struct do_nothing
{
    template<typename T>
    void add (const T &) { }
    template<typename T>
    void remove (const T &) { }
};

/// @brief A sliding window of samples
///
/// @tparam T sample type
template<typename T>
class sliding_window
{
    private:
    /// @brief window duration
    uint64_t duration;
    /// @brief two deque add/remove in lock-step
    std::deque<uint64_t> timestamps;
    std::deque<T> samples;
    public:
    /// @brief constructor
    ///
    /// @param duration duration of the window in useconds
    sliding_window (uint64_t duration)
        : duration (duration)
    {
    }
    /// @brief size of container
    ///
    /// @return the size
    size_t size () const
    {
        return samples.size ();
    }
    /// @brief indicates how full the buffer is
    ///
    /// @return 0.0 for empty, 1.0 for full
    float fullness (uint64_t ts) const
    {
        if (timestamps.empty ())
            return 0.0;
        assert (ts >= timestamps.back ());
        return static_cast<float> (ts - timestamps.back ()) / duration;
    }
    /// @brief container access
    ///
    /// @return sliding window container
    const std::deque<T> &get_samples () const
    {
        return samples;
    }
    /// @brief set the window duration
    ///
    /// @param d
    void set_duration (uint64_t d)
    {
        duration = d;
    }
    /// @brief remove all samples from the window
    void clear ()
    {
        timestamps.clear ();
        samples.clear ();
    }
    /// @brief add a sample
    ///
    /// @tparam U observer type
    /// @param ts timestamp in useconds
    /// @param s the sample
    /// @param obs observer
    template<typename U>
    void add (uint64_t ts, const T &s, U &obs)
    {
        assert (timestamps.size () == samples.size ());
        // add it
        timestamps.push_front (ts);
        samples.push_front (s);
        // signal it was added
        obs.add (s);
        // remove samples with old timestamps
        while (!samples.empty ())
        {
            assert (ts >= timestamps.back ());
            // any more old samples?
            if (ts - timestamps.back () >= duration)
            {
                // signal it was removed
                obs.remove (samples.back ());
                // remove it
                timestamps.pop_back ();
                samples.pop_back ();
            }
            else
                break;
        }
    }
    /// @brief add a sample
    ///
    /// @param ts timestamp in useconds
    /// @param s the sample
    void add (uint64_t ts, const T &s)
    {
        do_nothing dummy;
        add (ts, s, dummy);
    }
};

}

#endif
