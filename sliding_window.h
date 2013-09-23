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
#include <deque>

namespace soma
{

/// @brief A sliding window of samples
///
/// @tparam T sample type
template<typename T>
class sliding_window
{
    private:
    const uint64_t duration;
    std::deque<std::pair<uint64_t,T>> samples;
    void update (uint64_t ts)
    {
        // remove samples with old timestamps
        while (!samples.empty ())
        {
            assert (ts >= samples.back ().first);
            // any more old samples?
            if (ts - samples.back ().first >= duration)
            {
                // signal that it is being removed
                samples.back ().second.remove ();
                // remove it
                samples.pop_back ();
            }
            else
                break;
        }
    }
    public:
    /// @brief constructor
    ///
    /// @param duration duration of the window in useconds
    sliding_window (uint64_t duration)
        : duration (duration)
    {
    }
    /// @brief remove all samples from the window
    void clear ()
    {
        for (auto i : samples)
            // signal that it is being removed
            i.second.remove ();
        samples.clear ();
    }
    /// @brief get the number of samples in the deque
    ///
    /// @return size
    size_t size () const
    {
        return samples.size ();
    }
    /// @brief add a sample
    ///
    /// @param ts timestamp in useconds
    /// @param s the sample
    void add (uint64_t ts, const T &s)
    {
        // signal that it is being added
        s.add ();
        // add it
        samples.emplace_front (make_pair (ts, s));
        // remove samples with old timestamps
        update (ts);
    }
};

}

#endif
