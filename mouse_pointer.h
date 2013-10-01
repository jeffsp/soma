/// @file mouse_pointer.h
/// @brief keep track of mouse position
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-01

#ifndef MOUSE_POINTER_H
#define MOUSE_POINTER_H

#include "hand_sample.h"
#include "mouse.h"
#include "sliding_window.h"
#include "stats.h"

namespace soma
{

class mouse_pointer
{
    private:
    static const uint64_t SW_DURATION = 50000;
    sliding_window<int> swx;
    sliding_window<int> swy;
    running_mean x;
    running_mean y;
    mouse &m;
    double speed;
    bool last_valid;
    double last_x;
    double last_y;
    public:
    mouse_pointer (mouse &m, double speed)
        : swx (SW_DURATION)
        , swy (SW_DURATION)
        , m (m)
        , speed (speed)
        , last_valid (false)
    {
    }
    void set_speed (double s)
    {
        if (s >= 1.0)
            speed = s;
    }
    void clear ()
    {
        swx.clear ();
        swy.clear ();
        x.reset ();
        y.reset ();
        last_valid = false;
    }
    void update (uint64_t ts, const vec3 &pos)
    {
        swx.add (ts, pos.x, x);
        swy.add (ts, pos.y, y);
        if (last_valid)
        {
            double px = last_x - x.get_mean ();
            double py = last_y - y.get_mean ();
            m.move (-px * speed, py * speed);
        }
        last_valid = true;
        last_x = x.get_mean ();
        last_y = y.get_mean ();
    }
    void center ()
    {
        m.center ();
    }
};

}

#endif
