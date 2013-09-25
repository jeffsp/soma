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

class soma_mouse : public Leap::Listener
{
    private:
    bool done;
    const options &opts;
    const hand_shape_classifier &hsc;
    static const uint64_t SWHS_DURATION = 100000;
    sliding_window<hand_sample> swhs;
    mouse_pointer mp;
    frame_counter fc;
    mouse m;
    timestamps ts;
    void update (const hand_shape shape, const hand_sample &hs)
    {
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
    public:
    soma_mouse (const options &opts, const hand_shape_classifier &hsc)
        : done (false)
        , opts (opts)
        , hsc (hsc)
        , swhs (SWHS_DURATION)
        , mp (m, opts.get_mouse_speed ())
    {
    }
    ~soma_mouse ()
    {
        std::clog << fc.fps () << "fps" << std::endl;
    }
    bool is_done () const
    {
        return done;
    }
    virtual void onFrame (const Leap::Controller& c)
    {
        // get the frame
        Leap::Frame f = c.frame ();
        uint64_t ts = f.timestamp ();
        // get the sample
        hand_sample hs (f.pointables ());
        // add it to the window
        swhs.add (ts, hs);
        // update frame counter
        fc.update (ts);
        // only operate when we have some samples
        if (swhs.size () < 5)
        {
            update (-1, hs);
            return;
        }
        // TODO should get using running mode to get nfingers
        hand_samples s;
        for (auto i : swhs.get_samples ())
            s.push_back (i.second);
        // did we get anything?
        if (s.empty ())
        {
            update (-1, hs);
            return;
        }
        // filter out bad samples
        hand_samples fs = filter (s);
        const size_t nf = s.size () - fs.size ();
        // make sure they are reliable samples
        if (100 * nf / s.size () > 25) // more than 25%?
        {
            update (-1, hs);
            return;
        }
        // end if you show 6 or more fingers
        if (!fs.empty () && fs[0].size () > 5)
        {
            update (-1, hs);
            done = true;
            return;
        }
        // convert them to feature vectors
        std::vector<hand_shape_features> fv (fs.begin (), fs.end ());
        // classify them
        std::map<hand_shape,double> l;
        hand_shape shape = hsc.classify (fv, l);
        update (shape, hs);
    }
};

}

#endif
