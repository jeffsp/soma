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
    static const unsigned RADIUS = 50;
    sliding_window<int> swx;
    sliding_window<int> swy;
    running_mean smooth_x;
    running_mean smooth_y;
    mouse &m;
    double speed;
    bool last_valid;
    vec3 last;
    bool centered;
    vec3 cp;
    void update_normal (uint64_t ts, const vec3 &pos)
    {
        swx.add (ts, pos.x, smooth_x);
        swy.add (ts, pos.y, smooth_y);
        double x = smooth_x.get_mean ();
        double y = smooth_y.get_mean ();
        if (last_valid)
        {
            double px = last.x - x;
            double py = last.y - y;
            m.move (-px * speed, py * speed);
        }
        last_valid = true;
        last.x = x;
        last.y = y;
    }
    public:
    mouse_pointer (mouse &m, double speed)
        : swx (SW_DURATION)
        , swy (SW_DURATION)
        , m (m)
        , speed (speed)
        , last_valid (false)
        , centered (false)
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
        last_valid = false;
    }
    void update (uint64_t ts, const vec3 &pos)
    {
        double dx = cp.x - pos.x;
        double dy = cp.y - pos.y;
        double r = sqrt (dx * dx + dy * dy);
        if (r < RADIUS || !centered)
        {
            // normal mode
            update_normal (ts, pos);
        }
        else
        {
            // joystick mode
            if (r < RADIUS * 1.2)
                return;
            if (!last_valid)
                update_normal (ts, pos);
            else
            {
                double theta = atan2 (dy, dx);
                double x = 10.0 * cos (theta);
                double y = 10.0 * sin (theta);
                std::clog << theta << " " << x << " " << y << std::endl;
                vec3 tmp;
                tmp.x = last.x - x;
                tmp.y = last.y - y;
                update_normal (ts, tmp);
            }
        }
        // else do nothing
    }
    void center (const vec3 &pos)
    {
        centered = !centered;
        std::clog << "center " << (centered ? "ON" : "OFF") << std::endl;
        cp = pos;
    }
};

}

#endif
