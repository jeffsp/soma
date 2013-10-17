/// @file mouse_pointer.h
/// @brief keep track of mouse position
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-01

#ifndef MOUSE_POINTER_H
#define MOUSE_POINTER_H

#include "touch_port.h"
#include <fstream>

namespace soma
{

class mouse_pointer
{
    private:
    static const uint64_t SW_DURATION = 50000;
    sliding_window<double> swx;
    sliding_window<double> swy;
    running_mean smooth_x;
    running_mean smooth_y;
    mouse &m;
    double speed;
    touch_port tp;
    public:
    mouse_pointer (mouse &m, double speed)
        : swx (SW_DURATION)
        , swy (SW_DURATION)
        , m (m)
        , speed (speed)
    {
        tp.set (vec3 (-150, 300, 0), vec3 (80, 301, 0), vec3 (-130, 160, 0), vec3 (90, 165, 0));
        tp.write (std::clog);
        tp.set_screen_dimensions (m.width (), m.height ());
    }
    void set_speed (double s)
    {
        if (s > 0.0)
            speed = s;
    }
    void recenter (const vec3 &pos)
    {
        tp.recenter (pos);
    }
    void clear ()
    {
        swx.clear ();
        swy.clear ();
        smooth_x.reset ();
        smooth_y.reset ();
    }
    void update (const uint64_t ts, const vec3 &pos)
    {
        swx.add (ts, pos.x, smooth_x);
        swy.add (ts, pos.y, smooth_y);
        double x = smooth_x.get_mean ();
        double y = smooth_y.get_mean ();
        m.set (tp.mapx (x), tp.mapy (y));
    }
};

}

#endif
