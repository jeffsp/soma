/// @file time_guard.h
/// @brief place a guard on how fast an event can occur
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-17

#ifndef TIME_GUARD_H
#define TIME_GUARD_H

#include <cassert>
#include <cstdint>

namespace soma
{

class time_guard
{
    private:
    uint64_t ts;
    uint64_t duration;
    public:
    time_guard ()
        : ts (0)
        , duration (0)
    {
    }
    void turn_on (uint64_t t, uint64_t d)
    {
        ts = t;
        duration = d;
    }
    bool is_on (uint64_t t) const
    {
        assert (t >= ts);
        return (t - ts) < duration;
    }
    void reset ()
    {
        ts = 0;
        duration = 0;
    }
};

}

#endif
