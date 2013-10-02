/// @file frame_counter.h
/// @brief frame_counter class
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-23

#ifndef FRAME_COUNTER_H
#define FRAME_COUNTER_H

#include <cstdint>
#include <unistd.h>

namespace soma
{

/// @brief keep track of frames and the time it takes to display them
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
    /// @brief update the counter
    ///
    /// @param ts timestamp of the frame in usecs
    void update (uint64_t ts)
    {
        if (frames == 0)
            first_ts = ts;
        else
            last_ts = ts;
        ++frames;
    }
    /// @brief get number of frames counted
    ///
    /// @return total frames counted
    uint64_t get_frames () const
    {
        return frames;
    }
    /// @brief get frames per second
    ///
    /// @return fps
    double fps () const
    {
        double secs = static_cast<double> (last_ts - first_ts) / 1000000;
        // don't count the last frame
        if (frames != 0 && secs != 0.0f)
            return (frames - 1) / secs;
        return -1;
    }
};

}

#endif
