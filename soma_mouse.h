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

#include "keyboard.h"
#include "mouse_pointer.h"
#include "options.h"
#include "soma.h"
#include <unistd.h>

namespace soma
{

class time_guard
{
    private:
    uint64_t ts;
    uint64_t duration;
    public:
    time_guard ()
        : ts (0)
        , duration (0)
    {
    }
    void turn_on (uint64_t t, uint64_t d)
    {
        ts = t;
        duration = d;
    }
    bool guarded (uint64_t t)
    {
        assert (t >= ts);
        return (t - ts) < duration;
    }
};

class soma_mouse : public Leap::Listener
{
    private:
    static const uint64_t CLICK_GUARD_DURATION = 300000;
    bool done;
    const options &opts;
    hand_shape_classifier hsc;
    mouse_pointer mp;
    frame_counter fc;
    mouse m;
    keyboard k;
    time_guard can_click;
    void check_click (uint64_t ts)
    {
        if (can_click.guarded (ts))
            return;
        std::vector<int> s = k.key_states ();
        // was shift pressed?
        if (s[0] || s[1])
        {
            // guard it so we don't click mutliple times
            can_click.turn_on (ts, CLICK_GUARD_DURATION);
            // left click down
            m.click (1, 1);
            // wait to release
            usleep (10000);
            // left click up
            m.click (1, 0);
        }
        // was alt pressed?
        else if (s[2] || s[3])
        {
            // guard it so we don't click mutliple times
            can_click.turn_on (ts, CLICK_GUARD_DURATION);
            // right click down
            m.click (3, 1);
            // wait to release
            usleep (10000);
            // right click up
            m.click (3, 0);
        }
    }
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
            {
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
                check_click (ts);
            }
            case hand_shape::clicking:
            break;
            case hand_shape::scrolling:
            mp.clear ();
            break;
            case hand_shape::ok:
            mp.clear ();
            break;
            case hand_shape::center:
            m.set (m.width () / 2, m.height () / 2);
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
