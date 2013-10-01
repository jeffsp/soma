/// @file finger_counter.h
/// @brief finger_counter class
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-23

#ifndef FINGER_COUNTER_H
#define FINGER_COUNTER_H

#include "sliding_window.h"
#include "stats.h"
#include <cstdint>

namespace soma
{

/// @brief count fingers over a sliding window
class finger_counter
{
    private:
    /// @brief the window
    sliding_window<int> sw;
    /// @brief current finger count
    int current;
    /// @brief flag if number has changed
    bool changed;
    /// @brief indicates how full the window must be
    float window_fullness;
    /// @brief indicates how many of the samples must be equal to the mode
    float mode_ratio;
    /// @brief mode of finger count
    running_mode rm;
    public:
    /// @brief constructor
    ///
    /// @param duration window duration
    /// @param window_fullness
    /// @param mode_ratio
    finger_counter (uint64_t duration,
            float window_fullness = 0.5,
            float mode_ratio = 0.3)
        : sw (duration)
        , current (-1)
        , changed (false)
        , window_fullness (window_fullness)
        , mode_ratio (mode_ratio)
    {
    }
    /// @brief add a samples
    ///
    /// @param ts timestamp
    /// @param nfingers sample
    void add (uint64_t ts, int nfingers)
    {
        // remember previous
        int last = current;
        // update window
        sw.add (ts, nfingers, rm);
        // is the sample window full enough?
        if (sw.fullness (ts) < window_fullness)
            current = -1;
        // does the mode represent enough samples?
        else if (static_cast<float> (rm.get_count ()) / sw.size () < mode_ratio)
            current = -1;
        else
            current = rm.get_mode ();
        // set changed flag
        changed = (last != current);
    }
    /// @brief get current count
    ///
    /// @return the count or -1 if we are not sure
    ///
    /// We have to have enough samples in the window to be sure about the count.
    int get_count () const
    {
        return current;
    }
    /// @brief flag if the last sample changed the count
    ///
    /// @return true if changed
    bool has_changed () const
    {
        return changed;
    }
};

}

#endif
