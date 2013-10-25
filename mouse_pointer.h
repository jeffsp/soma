/// @file mouse_pointer.h
/// @brief keep track of mouse position
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-01

#ifndef MOUSE_POINTER_H
#define MOUSE_POINTER_H

namespace soma
{

double mm_to_pixels (const double mm)
{
    return mm * 3.7795;
}

class mouse_pointer
{
    private:
    static const uint64_t SW_DURATION = 50000;
    sliding_window<double> swx;
    sliding_window<double> swy;
    running_mean smooth_x;
    running_mean smooth_y;
    point_delta<vec3> dd;
    mouse &m;
    double speed;
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
        if (s > 0.0)
            speed = s;
    }
    void center ()
    {
        m.set (m.width () / 2, m.height () / 2);
    }
    void clear ()
    {
        swx.clear ();
        swy.clear ();
        smooth_x.reset ();
        smooth_y.reset ();
    }
    void update (const uint64_t ts, const hand_sample &s)
    {
        if (s.size () == 2 || s.size () == 1)
        {
            const double MIND = 40;
            const double MAXD = 100;
            vec3 p = s[0].position;
            double d = MIND;
            if (s.size () == 2)
            {
                p = s[1].position;
                d = s[0].position.distanceTo (s[1].position);
            }
            const double x = p.x;
            const double y = p.y;
            swx.add (ts, x, smooth_x);
            swy.add (ts, y, smooth_y);
            const double sx = smooth_x.get_mean ();
            const double sy = smooth_y.get_mean ();
            // get index pointer
            dd.update (ts, vec3 (sx, sy, 0));
            const double dx = dd.current ().x - dd.last ().x;
            const double dy = dd.last ().y - dd.current ().y;
            const double MING = 0.5;
            const double MAXG = 5.0;
            double gain = (d - MIND) / MAXD;
            gain = gain < 0.0 ? 0.0 : gain;
            gain = gain > 1.0 ? 1.0 : gain;
            gain = gain * (MAXG - MING) + MING;
            // make sure time delta is a reasonable value
            if (dd.dt () == 0 || dd.dt () > 500000)
                return;
            // make it independent of framerate
            const double fr = 1000000.0 / dd.dt ();
            const double mx = dx * 100 / fr;
            const double my = dy * 100 / fr;
            // convert to pixels
            const double px =  mm_to_pixels (mx);
            const double py =  mm_to_pixels (my);
            //std::clog << fr << '\t' << px << '\t' << py << std::endl;
            m.move (gain * px, gain * py);
        }
    }
};

}

#endif
