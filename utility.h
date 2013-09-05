/// @file utility.h
/// @brief utility routines
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-05

#ifndef UTILITY_H
#define UTILITY_H

#include <cstdint>

namespace soma
{

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
    float fps () const
    {
        float secs = static_cast<float> (last_ts - first_ts) / 1000000;
        // don't count the last frame
        if (frames != 0 && secs != 0.0f)
            return (frames - 1) / secs;
        return -1;
    }
};

}

#endif
