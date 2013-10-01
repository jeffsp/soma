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

#include "mouse_pointer.h"
#include "options.h"
#include "soma.h"

namespace soma
{

class soma_mouse : public Leap::Listener
{
    private:
    bool done;
    const options &opts;
    hand_shape_classifier hsc;
    mouse_pointer mp;
    frame_counter fc;
    mouse m;
    void update (uint64_t ts, const hand_shape shape, const hand_sample &s)
    {
        switch (shape)
        {
            default:
            assert (0); // logic error
            break;
            case hand_shape::unknown:
            mp.clear ();
            break;
            case hand_shape::zero:
            mp.clear ();
            break;
            case hand_shape::pointing:
            case hand_shape::clicking:
            {
                if (!s.empty ())
                {
                    hand_sample tmp (s);
                    sort (tmp.begin (), tmp.end (), sort_top_to_bottom);
                    mp.update (ts, tmp[0].position);
                }
            }
            break;
            case hand_shape::scrolling:
            mp.clear ();
            break;
            case hand_shape::ok:
            mp.clear ();
            break;
            case hand_shape::center:
            mp.clear ();
            mp.center ();
            break;
        }
    }
    public:
    soma_mouse (const options &opts)
        : done (false)
        , opts (opts)
        , hsc (200000)
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
        if (done)
            return;
        // get the frame
        Leap::Frame f = c.frame ();
        uint64_t ts = f.timestamp ();
        // update frame counter
        fc.update (ts);
        // get the sample
        hand_sample s (f.pointables ());
        // quit?
        if (s.size () > 5)
        {
            done = true;
            return;
        }
        // add it to the classifier
        hsc.add (ts, s);
        // update the mouse
        update (ts, hsc.get_shape (), s);
    }
};

}

#endif
