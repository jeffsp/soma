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

double quadratic (const double a, const double b, const double c)
{
    double z = b * b - 4 * a * c;
    return (-b + sqrt (abs (z))) / (2 * a);
}

class touchport
{
    private:
    vec3 tl;
    vec3 tr;
    vec3 bl;
    vec3 br;
    vec3 offset;
    int width;
    int height;
    // get coordinates in 0, 1
    double map (const double x, const double A, const double B, const double C, const double D) const
    {
        const double a = A - B - C + D;
        const double b = -2 * A + B + C;
        const double c = A - x;
        const double y = quadratic (a, b, c);
        return y;
    }
    public:
    void set_screen_dimensions (int w, int h)
    {
        width = w;
        height = h;
    }
    void set_center (const vec3 &p)
    {
        offset = vec3 (0, 0, 0);
        const double x1 = mapx (p.x);
        const double y1 = mapy (p.y);
        double cx = (tl.x + tr.x + bl.x + br.x) / 4.0;
        double cy = (tl.y + tr.y + bl.y + br.y) / 4.0;
        const double x2 = mapx (cx);
        const double y2 = mapy (cy);
        offset.x = x2 - x1;
        offset.y = y2 - y1;
        std::clog << "offset " << offset << std::endl;
    }
    void read (std::istream &s)
    {
        s >> tl;
        s >> tr;
        s >> bl;
        s >> br;
    }
    void write (std::ostream &s) const
    {
        s << tl << std::endl;
        s << tr << std::endl;
        s << bl << std::endl;
        s << br << std::endl;
    }
    int mapx (double x) const
    {
        const double A = std::min (tl.x, bl.x);
        const double B = std::max (tl.x, bl.x);
        const double C = std::min (tr.x, br.x);
        const double D = std::max (tr.x, br.x);
        const double sx = map (x, A, B, C, D);
        //std::clog << "sx " << sx << std::endl;
        //std::clog << "x " << (1 - sx) * width << std::endl;
        return sx * width + offset.x;
    }
    int mapy (double y) const
    {
        // interpolate to get 3d coord mapped into screen coordinate
        const double A = std::min (tl.y, bl.y);
        const double B = std::max (tl.y, bl.y);
        const double C = std::min (tr.y, br.y);
        const double D = std::max (tr.y, br.y);
        const double sy = map (y, A, B, C, D);
        //std::clog << "y " << y << " sy " << sy << std::endl;
        //std::clog << "y " << (2 + sy) * height << std::endl;
        return  (1 - sy) * height + offset.y;
    }
};

class mouse_pointer
{
    private:
    static const uint64_t SW_DURATION1 = 20000;
    static const uint64_t SW_DURATION2 = 30000;
    sliding_window<double> swx;
    sliding_window<double> swy;
    sliding_window<double> swxv1;
    sliding_window<double> swyv1;
    sliding_window<double> swxv2;
    sliding_window<double> swyv2;
    running_mean smooth_x;
    running_mean smooth_y;
    running_mean smooth_xv1;
    running_mean smooth_yv1;
    running_mean smooth_xv2;
    running_mean smooth_yv2;
    mouse &m;
    double speed;
    touchport tp;
    int mode;
    point_delta pd;
    public:
    mouse_pointer (mouse &m, double speed)
        : swx (SW_DURATION1)
        , swy (SW_DURATION1)
        , swxv1 (SW_DURATION1)
        , swyv1 (SW_DURATION1)
        , swxv2 (SW_DURATION1)
        , swyv2 (SW_DURATION1)
        , m (m)
        , speed (speed)
    {
        std::string fn ("touchport.txt");
        std::clog << "opening " << fn << std::endl;
        std::ifstream ifs (fn.c_str ());
        if (!ifs)
            throw std::runtime_error ("could not open file");
        tp.read (ifs);
        tp.write (std::clog);
        tp.set_screen_dimensions (m.width (), m.height ());
    }
    void set_center (const vec3 &p)
    {
        tp.set_center (p);
    }
    void set_speed (double s)
    {
        if (s > 0.0)
            speed = s;
    }
    void set_mode (int m)
    {
        mode = m;
    }
    void clear ()
    {
        swx.clear ();
        swy.clear ();
        swxv1.clear ();
        swyv1.clear ();
        swxv2.clear ();
        swyv2.clear ();
        smooth_x.reset ();
        smooth_y.reset ();
        smooth_xv1.reset ();
        smooth_yv1.reset ();
        smooth_xv2.reset ();
        smooth_yv2.reset ();
    }
    void update (const uint64_t ts, const vec3 &pos)
    {
        swx.add (ts, pos.x, smooth_x);
        swy.add (ts, pos.y, smooth_y);
        swxv1.add (ts, pos.x, smooth_xv1);
        swyv1.add (ts, pos.y, smooth_yv1);
        swxv2.add (ts, pos.x * pos.x, smooth_xv2);
        swyv2.add (ts, pos.y * pos.y, smooth_yv2);
        double x = smooth_x.get_mean ();
        double y = smooth_y.get_mean ();
        double xv1 = smooth_xv1.get_mean ();
        double yv1 = smooth_yv1.get_mean ();
        double xv2 = smooth_xv2.get_mean ();
        double yv2 = smooth_yv2.get_mean ();
        // get variance
        double xv = xv2 - xv1 * xv1;
        double yv = yv2 - yv1 * yv1;
        //std::clog << xv << '\t' << yv << std::endl;
        if (xv + yv > 10)
        {
            swx.set_duration (SW_DURATION1);
            swy.set_duration (SW_DURATION1);
        }
        else
        {
            swx.set_duration (SW_DURATION2);
            swy.set_duration (SW_DURATION2);
        }
        pd.update (ts, x, y);
        if (mode == 0)
            m.set (tp.mapx (x), tp.mapy (y));
        else
            m.move (speed * mm_to_pixels (pd.dx ()), -speed * mm_to_pixels (pd.dy ()));
    }
};

}

#endif
