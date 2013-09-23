/// @file soma_mouse.h
/// @brief soma mouse implementation
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-05

#ifndef SOMA_MOUSE_H
#define SOMA_MOUSE_H

/// @brief version info
const int MAJOR_REVISION = 0;
const int MINOR_REVISION = 1;

#include "mouse.h"
#include "options.h"
#include "soma.h"

namespace soma
{

class mouse_pointer
{
    private:
    mouse &m;
    vec3 last_point;
    bool last_point_valid;
    double speed;
    public:
    mouse_pointer (mouse &m, double speed)
        : m (m)
        , last_point_valid (false)
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
        last_point_valid = false;
    }
    void update (const vec3 &p)
    {
        if (last_point_valid)
        {
            double x = last_point.x - p.x;
            double y = last_point.y - p.y;
            m.move (-x * speed, y * speed);
        }
        last_point = p;
        last_point_valid = true;
    }
    void center ()
    {
        m.center ();
    }
};

class soma_mouse
{
    private:
    const options &opts;
    frame_counter fc;
    mouse m;
    mouse_pointer mp;
    public:
    soma_mouse (const options &opts)
        : opts (opts)
        , mp (m, opts.get_mouse_speed ())
    {
    }
    ~soma_mouse ()
    {
        std::clog << fc.fps () << "fps" << std::endl;
    }
    void update (uint64_t ts, const hand_shape shape, const hand_sample &hs)
    {
        fc.update (ts);
        switch (shape)
        {
            default:
            assert (0);
            break;
            case -1:
            mp.clear ();
            break;
            case 0:
            case 1:
            {
                hand_sample tmp (hs);
                assert (!tmp.empty ());
                sort (tmp.begin (), tmp.end (), sort_top_to_bottom);
                mp.update (tmp[0].position);
            }
            break;
            case 2:
            break;
            case 3:
            break;
            case 4:
            mp.clear ();
            mp.center ();
            break;
        }
    }
};

}

#endif
