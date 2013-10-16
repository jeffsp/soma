/// @file mouse_scroller.h
/// @brief scroll up and down
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-16

#ifndef MOUSE_SCROLLER_H
#define MOUSE_SCROLLER_H

namespace soma
{

class mouse_scroller
{
    private:
    static const uint64_t SW_DURATION = 50000;
    sliding_window<double> swx;
    sliding_window<double> swy;
    running_mean smooth_x;
    running_mean smooth_y;
    mouse &m;
    double speed;
    public:
    mouse_scroller (mouse &m, double speed = 1.0)
        : swx (SW_DURATION)
        , swy (SW_DURATION)
        , m (m)
        , speed (speed)
    {
    }
    void set_speed (double s)
    {
        if (s > 0.0)
            speed = s;
    }
    void clear ()
    {
        swx.clear ();
        swy.clear ();
        smooth_x.reset ();
        smooth_y.reset ();
    }
    void update (const uint64_t ts, const vec3 &pos1, const vec3 &pos2)
    {
        swx.add (ts, pos1.x, smooth_x);
        swy.add (ts, pos1.y, smooth_y);
        //double x = smooth_x.get_mean ();
        //double y = smooth_y.get_mean ();
    }
};

}

#endif
