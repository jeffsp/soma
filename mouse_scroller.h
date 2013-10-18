/// @file mouse_scroller.h
/// @brief scroll up and down
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-16

#ifndef MOUSE_SCROLLER_H
#define MOUSE_SCROLLER_H

#include "point_delta.h"
#include "time_guard.h"

namespace soma
{

template<typename T>
struct vec2
{
    T x, y;
    vec2 (T x, T y) : x (x), y (y) { }
    vec2 () : x (0), y (0) { }
};

class mouse_scroller
{
    private:
    static const uint64_t SW_DURATION = 50000;
    sliding_window<double> swy;
    running_mean smooth_y;
    point_delta<double> dy;
    point_delta<double> dd;
    mouse &m;
    double speed;
    double min_distance;
    time_guard can_click;
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
        double d = pos1.distanceTo (pos2);
        // if the distance is too great, ignore it
        if (d > min_distance)
            return;
        // save the distance between the points
        dd.update (ts, d);
        // if they are moving away from one another, ignore it
        // TODO convert to mm/sec
        if (dd.current () - dd.last () > 0.3)
            return;
        swy.add (ts, pos1.y, smooth_y);
        double y = smooth_y.get_mean ();
        // is it moving up or down?
        dy.update (ts, y);
        // TODO convert to mm/sec
        if (fabs (dy.current () - dy.last ()) < 0.1)
            return;
        // check the guard
        if (can_click.is_on (ts))
            return;
        // place a guard
        // TODO make a parameter
        can_click.turn_on (ts, 100000);
        // scroll
        if (dy.current () - dy.last () > 0)
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
