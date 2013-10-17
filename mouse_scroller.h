/// @file mouse_scroller.h
/// @brief scroll up and down
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-16

#ifndef MOUSE_SCROLLER_H
#define MOUSE_SCROLLER_H

#include "point_delta.h"

namespace soma
{

class mouse_scroller
{
    private:
    point_delta d;
    static const uint64_t SW_DURATION = 50000;
    sliding_window<double> swy;
    running_mean smooth_y;
    mouse &m;
    double speed;
    double min_distance;
    public:
    mouse_scroller (mouse &m, double speed = 1.0)
        : swy (SW_DURATION)
        , m (m)
        , speed (speed)
        , min_distance (55)
    {
    }
    void set_speed (double s)
    {
        if (s > 0.0)
            speed = s;
    }
    void clear ()
    {
        swy.clear ();
        smooth_y.reset ();
    }
    void update (const uint64_t ts, const vec3 &pos1, const vec3 &pos2)
    {
        if (fabs (pos1.x - pos2.x) > min_distance)
            return;
        swy.add (ts, pos1.y, smooth_y);
        double y = smooth_y.get_mean ();
        d.update (ts, pos1.x, y);
        double dy = d.dy ();
        if (fabs (dy) < 0.1)
            return;
        if (d.dy () > 0)
        {
            m.click (4, 1);
            m.click (4, 0);
        }
        else
        {
            m.click (5, 1);
            m.click (5, 0);
        }
    }
};

}

#endif
