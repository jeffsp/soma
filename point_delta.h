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
class point_delta
{
    private:
    bool valid;
    uint64_t max_dt;
    T last_p;
    T current_p;
    uint64_t last_t;
    uint64_t current_t;
    public:
    point_delta (uint64_t max_dt = 100000)
        : valid (false)
        , max_dt (max_dt)
    {
    }
    void update (uint64_t ts, const T &x)
    {
        if (dt () > max_dt)
            reset ();
        if (valid)
        {
            last_p = current_p;
            last_t = current_t;
        }
        else
        {
            last_p = x;
            last_t = ts;
        }
        current_p = x;
        current_t = ts;
        valid = true;
    }
    const T &current () const
    {
        return current_p;
    }
    const T &last () const
    {
        return last_p;
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
