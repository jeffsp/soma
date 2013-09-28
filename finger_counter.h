/// @file finger_counter.h
/// @brief finger_counter class
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-23

#ifndef FINGER_COUNTER_H
#define FINGER_COUNTER_H

#include "soma.h"
#include <cstdint>

namespace soma
{

/// @brief count fingers over a sliding window
class finger_counter
{
    private:
    sliding_window<int> w;
    running_mode m;
    public:
    finger_counter (uint64_t duration)
        : w (duration)
    {
    }
    void add (uint64_t ts, int nfingers)
    {
        w.add (ts, nfingers, m);
    }
    size_t count () const
    {
        return m.mode ();
    }
};

}

#endif
