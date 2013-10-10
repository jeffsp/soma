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

template<typename T>
struct vec2
{
    T x, y;
    vec2 (T x, T y) : x (x), y (y) { }
    vec2 () : x (0), y (0) { }
};

// convert from mm to pixels at 96dpi
double mm_to_pixels (double mm)
{
    return mm * 3.779527559;
}

class point_delta
{
    private:
    bool valid;
    vec2<double> last_p;
    vec2<double> current_p;
    uint64_t last_t;
    uint64_t current_t;
    public:
    point_delta ()
        : valid (false)
    { }
    void update (uint64_t ts, const double x, const double y)
    {
        if (valid)
        {
            last_p = current_p;
            last_t = current_t;
        }
        else
        {
            last_p = vec2<double> (x, y);
            last_t = ts;
        }
        current_p = vec2<double> (x, y);
        current_t = ts;
        valid = true;
    }
    double dx () const
    {
        return current_p.x - last_p.x;
    }
    double dy () const
    {
        return current_p.y - last_p.y;
    }
    double dr () const
    {
        return sqrt (dx () * dx () + dy () * dy ());
    }
    uint64_t dt () const
    {
        assert (current_t >= last_t);
        return current_t - last_t;
    }
    void reset ()
    {
        valid = false;
    }
};

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
    point_delta delta;
    public:
    mouse_pointer (mouse &m, double speed)
        : swx (SW_DURATION)
        , swy (SW_DURATION)
        , m (m)
        , speed (speed)
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
        smooth_x.reset ();
        smooth_y.reset ();
        delta.reset ();
    }
    void update (uint64_t ts, const vec3 &pos)
    {
        swx.add (ts, pos.x, smooth_x);
        swy.add (ts, pos.y, smooth_y);
        double x = smooth_x.get_mean ();
        double y = smooth_y.get_mean ();
        delta.update (ts, x, y);
        // get mm per second
        double mmps = 1000000 * delta.dr () / delta.dt ();
        // did it move fast or slow?
        double px;
        double py;
        const int FAST = 200;
        if (mmps < FAST)
        {
            // slow
            px = mm_to_pixels (delta.dx ()) * speed;
            py = mm_to_pixels (-delta.dy ()) * speed;
        }
        else
        {
            // fast
            double gain = (mmps - FAST) / 100;
            px = gain * mm_to_pixels (delta.dx ()) * speed;
            py = gain * mm_to_pixels (-delta.dy ()) * speed;
        }
        m.move (px, py);
    }
};

}

#endif
