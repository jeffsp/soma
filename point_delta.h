/// @file point_delta.h
/// @brief keeps track of two adjacent 2D points
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-16

#ifndef POINT_DELTA_H
#define POINT_DELTA_H

namespace soma
{

template<typename T>
struct vec2
{
    T x, y;
    vec2 (T x, T y) : x (x), y (y) { }
    vec2 () : x (0), y (0) { }
};

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

}

#endif
