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
#include <fstream>

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

std::istream& operator>> (std::istream &s, vec3 &v)
{
    s >> v.x;
    s >> v.y;
    s >> v.z;
    return s;
}

class touchport
{
    private:
    vec3 tl;
    vec3 tr;
    vec3 bl;
    vec3 br;
    public:
    void read (std::istream &s)
    {
        s >> tl;
        s >> tr;
        s >> bl;
        s >> br;
    }
    void write (std::ostream &s)
    {
        s << tl << std::endl;
        s << tr << std::endl;
        s << bl << std::endl;
        s << br << std::endl;
    }
};

class mouse_pointer
{
    private:
    static const uint64_t SW_DURATION1 = 50000;
    static const uint64_t SW_DURATION2 = 200000;
    sliding_window<double> swx;
    sliding_window<double> swy;
    sliding_window<double> swv;
    running_mean smooth_x;
    running_mean smooth_y;
    running_mean smooth_v;
    mouse &m;
    double speed;
    point_delta delta;
    touchport tl;
    public:
    mouse_pointer (mouse &m, double speed)
        : swx (SW_DURATION1)
        , swy (SW_DURATION1)
        , swv (SW_DURATION2)
        , m (m)
        , speed (speed)
    {
        std::string fn ("touchport.txt");
        std::clog << "opening " << fn << std::endl;
        std::ifstream ifs (fn.c_str ());
        if (!ifs)
            throw std::runtime_error ("could not open file");
        tl.read (ifs);
        tl.write (std::clog);
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
        swv.clear ();
        smooth_x.reset ();
        smooth_y.reset ();
        smooth_v.reset ();
        delta.reset ();
    }
    void update (const uint64_t ts, const vec3 &pos)
    {
        swx.add (ts, pos.x, smooth_x);
        swy.add (ts, pos.y, smooth_y);
        double x = smooth_x.get_mean ();
        double y = smooth_y.get_mean ();
        delta.update (ts, x, y);
        double px = mm_to_pixels (delta.dx ()) * speed;
        double py = mm_to_pixels (-delta.dy ()) * speed;
        m.move (px, py);
        // get velocity in mm /sec
        if (delta.dt () != 0)
        {
            double v = 1000000 * delta.dr () / delta.dt ();
            swv.add (ts, v, smooth_v);
            double sv = std::min (smooth_v.get_mean (), 500.0);
            assert (sv >= 0.0);
            // slow pointer gets longer window
            uint64_t d = SW_DURATION1 + (500 - sv) * 250;
            swx.set_duration (d);
            swy.set_duration (d);
        }
    }
};

}

#endif
