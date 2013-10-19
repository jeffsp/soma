/// @file mouse_clicker.h
/// @brief detect mouse clicks
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-17

#ifndef MOUSE_CLICKER_H
#define MOUSE_CLICKER_H

#include "hand_sample.h"
#include "keyboard.h"
#include "mouse.h"
#include "point_delta.h"
#include "state_machine.h"
#include "time_guard.h"
#include <cassert>
#include <fstream>
#include <map>
#include <unistd.h>
#include <utility>

namespace soma
{

class pinch_detector
{
    private:
    // state machine definition
    enum class state { null, open, open_ready, closer, zero, closed, done };
    enum class event { open, closer, closed, zero, open_timer_done, closed_timer1_done, closed_timer2_done };
    static const uint64_t OPEN_TIMER_DURATION = 300000;
    static const uint64_t CLOSED_TIMER_DURATION1 = 300000;
    static const uint64_t CLOSED_TIMER_DURATION2 = 300000;
    static const int OPEN_MIN = 60;
    point_delta<double> dd;
    typedef void (pinch_detector::*member_function)(uint64_t);
    state_machine<state,event,member_function> sm;
    time_guard open_timer;
    time_guard closed_timer1;
    time_guard closed_timer2;
    public:
    // actions
    void reset (uint64_t ts)
    {
        sm.set_state (state::null);
        dd.reset ();
    }
    void start_open_timer (uint64_t ts)
    {
        open_timer.turn_on (ts, OPEN_TIMER_DURATION);
    }
    pinch_detector ()
    {
        sm.init (state::null, &pinch_detector::reset);
        //      state        event         action                            next state
        //      -----        -----         ------                            ----------
        sm.add (state::null, event::open, &pinch_detector::start_open_timer, state::open);
    }
    // public functions
    bool is_set () const
    {
        return sm.get_state () == state::done;
    }
    bool maybe () const
    {
        switch (sm.get_state ())
        {
            default:
            assert (0); // logic error
            case state::null:
            case state::open:
            case state::open_ready:
            return false;
            case state::closer:
            case state::zero:
            case state::closed:
            case state::done:
            return true;
        }
    }
    void update (const uint64_t ts, const hand_sample &s)
    {
        switch (s.size ())
        {
            default: // do nothing
            break;
            case 0:
            case 1: // d = 0.0
            sm.record (event::zero, *this, ts);
            break;
            case 2:
            {
                double d = s[0].position.distanceTo (s[1].position);
                dd.update (ts, d);
                if (d > OPEN_MIN)
                {
                    if (dd.current () < dd.last ())
                        sm.record (event::closer, *this, ts);
                    else
                        sm.record (event::open, *this, ts);
                }
                else // d < OPEN_MIN
                {
                    sm.record (event::closed, *this, ts);
                }
            }
        }
    }
};

class mouse_clicker
{
    private:
    static const uint64_t CLICK_GUARD_DURATION = 300000;
    time_guard can_click;
    pinch_detector pd;
    keyboard k;
    mouse &m;
    public:
    mouse_clicker (mouse &m)
        : m (m)
    {
    }
    void update (const uint64_t ts, const hand_sample &s)
    {
        hand_sample tmp (s);
        sort (tmp.begin (), tmp.end (), sort_left_to_right);
        pd.update (ts, tmp);
    }
    bool maybe_pinched (uint64_t ts) const
    {
        if (can_click.is_on (ts))
            return false;
        if (pd.maybe ())
            return true;
        return false;
    }
    bool did_pinch (uint64_t ts)
    {
        if (can_click.is_on (ts))
            return false;
        if (pd.is_set ())
        {
            pd.reset (ts);
            return true;
        }
        return false;
    }
    bool did_left_click (uint64_t ts) const
    {
        if (can_click.is_on (ts))
            return false;
        std::vector<int> s = k.key_states ();
        // was shift pressed?
        if (s[0] || s[1])
            return true;
        return false;
    }
    bool did_right_click (uint64_t ts) const
    {
        if (can_click.is_on (ts))
            return false;
        std::vector<int> s = k.key_states ();
        // was ctrl pressed?
        if (s[4] || s[5])
            return true;
        return false;
    }
    bool maybe_left_clicked (uint64_t ts) const
    {
        if (can_click.is_on (ts))
            return false;
        return false;
    }
    bool maybe_right_clicked (uint64_t ts) const
    {
        if (can_click.is_on (ts))
            return false;
        return false;
    }
    void pinch (uint64_t ts)
    {
        // guard it so we don't pinch multiple times
        can_click.turn_on (ts, CLICK_GUARD_DURATION);
    }
    void left_click (uint64_t ts)
    {
        // guard it so we don't click multiple times
        can_click.turn_on (ts, CLICK_GUARD_DURATION);
        // left click down
        m.click (1, 1);
        // wait to release
        usleep (10000);
        // left click up
        m.click (1, 0);
    }
    void right_click (uint64_t ts)
    {
        // guard it so we don't click multiple times
        can_click.turn_on (ts, CLICK_GUARD_DURATION);
        // right click down
        m.click (3, 1);
        // wait to release
        usleep (10000);
        // right click up
        m.click (3, 0);
    }
};

}

#endif
