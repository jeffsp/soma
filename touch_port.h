/// @file touch_port.h
/// @brief planar boundary in XY plane
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-16

#ifndef TOUCH_PORT_H
#define TOUCH_PORT_H

#include "hand_sample.h"
#include <iostream>

namespace soma
{

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

class touch_port
{
    private:
    vec3 tl;
    vec3 tr;
    vec3 bl;
    vec3 br;
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
    touch_port ()
    {
    }
    void set (const vec3 &tl, const vec3 &tr, const vec3 &bl, const vec3 &br)
    {
        this->tl = tl;
        this->tr = tr;
        this->bl = bl;
        this->br = br;
    }
    void set_screen_dimensions (int w, int h)
    {
        width = w;
        height = h;
    }
    void recenter (const vec3 &d)
    {
        tl.x += d.x; tl.y += d.y;
        bl.x += d.x; bl.y += d.y;
        tr.x += d.x; tr.y += d.y;
        br.x += d.x; br.y += d.y;
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
    double mapx (double x) const
    {
        const double A = std::min (tl.x, bl.x);
        const double B = std::max (tl.x, bl.x);
        const double C = std::min (tr.x, br.x);
        const double D = std::max (tr.x, br.x);
        return map (x, A, B, C, D);
    }
    double mapy (double y) const
    {
        // interpolate to get 3d coord mapped into screen coordinate
        const double A = std::min (tl.y, bl.y);
        const double B = std::max (tl.y, bl.y);
        const double C = std::min (tr.y, br.y);
        const double D = std::max (tr.y, br.y);
        return map (y, A, B, C, D);
    }
};

}

#endif
