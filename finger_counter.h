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
    sliding_window<int> w;
    /// @brief mode of finger count
    running_mode m;
    /// @brief flag if number has changed
    bool changed;
    public:
    /// @brief constructor
    ///
    /// @param duration window duration
    finger_counter (uint64_t duration)
        : w (duration)
        , changed (false)
    {
    }
    /// @brief add a samples
    ///
    /// @param ts timestamp
    /// @param nfingers sample
    void add (uint64_t ts, int nfingers)
    {
        int last = m.mode ();
        w.add (ts, nfingers, m);
        changed = (last != m.mode ());
    }
    /// @brief get current count
    ///
    /// @return the count
    size_t get_count () const
    {
        return m.mode ();
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
