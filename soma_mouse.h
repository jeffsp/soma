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
    bool done;
    const options &opts;
    hand_shape_classifier hsc;
    mouse m;
    mouse_pointer mp;
    mouse_scroller ms;
    mouse_clicker mc;
    frame_counter fc;
    bool stopped;
    time_guard can_changed_stopped;
    void update (uint64_t ts, const hand_shape shape, const hand_sample &s)
    {
        // if we are stopped and not trying to restart, return
        if (stopped && shape != hand_shape::stopping)
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
                // update the clicker
                mc.update (ts, s);
                if (mc.did_pinch (ts))
                {
                    mc.pinch (ts);
                    return;
                }
                if (mc.did_left_click (ts))
                {
                    mc.left_click (ts);
                    return;
                }
                if (mc.did_right_click (ts))
                {
                    mc.right_click (ts);
                    return;
                }
                // don't move the pointer if we might be in the middle of an action
                if (mc.maybe_pinched (ts))
                    return;
                if (mc.maybe_left_clicked (ts))
                    return;
                if (mc.maybe_right_clicked (ts))
                    return;
                // ok, move the pointer
                if (s.size () == 1)
                {
                    mp.update (ts, s[0].position);
                }
                else if (s.size () == 2)
                {
                    // get highest finger
                    hand_sample tmp (s);
                    sort (tmp.begin (), tmp.end (), sort_top_to_bottom);
                    mp.update (ts, tmp[0].position);
                }
            }
            break;

            case hand_shape::scrolling:
            {
                if (s.size () == 2)
                {
                    // thumb on left
                    hand_sample tmp (s);
                    sort (tmp.begin (), tmp.end (), sort_left_to_right);
                    ms.update (ts, tmp[0].position, tmp[1].position);
                }
                mp.clear ();
            }
            return;

            case hand_shape::stopping:
            {
                if (can_changed_stopped.guarded (ts))
                    return;
                // we are changing states
                stopped = !stopped;
                // guard the state
                can_changed_stopped.turn_on (ts, STOP_GUARD_DURATION);
                mp.clear ();
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
        , ms (m)
        , mc (m)
        , stopped (false)
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
