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

#include "options.h"
#include "soma.h"
#include <unistd.h>

namespace soma
{

class soma_mouse : public Leap::Listener
{
    private:
    static const uint64_t STOP_GUARD_DURATION = 2000000;
    static const uint64_t CENTER_DELAY_DURATION = 500000;
    bool done;
    const options &opts;
    hand_shape_classifier hsc;
    mouse m;
    mouse_pointer mp;
    mouse_clicker mc;
    mouse_scroller ms;
    frame_counter fc;
    time_guard is_centering;
    void update (uint64_t ts, const hand_shape shape, const hand_sample &s)
    {
        // if we are centering
        if (is_centering.is_on (ts))
            return;
        switch (shape)
        {
            default:
            assert (0); // logic error
            return;
            case hand_shape::unknown:
            mp.clear ();
            return;
            case hand_shape::zero:
            mp.clear ();
            return;
            case hand_shape::pointing:
            {
                if (mc.did_left_click (ts))
                {
                    mc.left_click (ts);
                    return;
                }
                mp.update (ts, s);
            }
            break;
            case hand_shape::scrolling:
            {
                if (s.size () == 3)
                {
                    // thumb on left
                    hand_sample tmp (s);
                    sort (tmp.begin (), tmp.end (), sort_left_to_right);
                    ms.update (ts, tmp[0].position, tmp[1].position);
                }
                mp.clear ();
            }
            return;
            case hand_shape::centering:
            {
                mp.center ();
                is_centering.turn_on (ts, CENTER_DELAY_DURATION);
            }
            return;
        }
    }
    public:
    soma_mouse (const options &opts)
        : done (false)
        , opts (opts)
        , hsc (200000)
        , mp (m, opts.get_mouse_speed ())
        , mc (m)
        , ms (m)
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
        if (s.size () > 6)
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
