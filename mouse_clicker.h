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
    // state machine definitions
    enum class state { start, open_waiting, open, zero, closed, accepted };
    enum class event { invalid, open, timer1, zero, closed, timer2 };
    typedef void (pinch_detector::*member_function)(uint64_t);
    state_machine<state,event,member_function> sm;
    // distinguish between open and closed
    static const int OPEN_MIN = 50;
    // timer support
    static const uint64_t TIMER1_DURATION = 300000;
    static const uint64_t TIMER2_DURATION = 600000;
    time_guard timer1;
    time_guard timer2;
    // determine is the fingers are open, but getting closer
    //point_delta<double> dd;
    public:
    // actions
    void do_nothing (uint64_t) { }
    void start_timer1 (uint64_t ts) { timer1.turn_on (ts, TIMER1_DURATION); }
    void start_timer2 (uint64_t ts) { timer2.turn_on (ts, TIMER2_DURATION); }
    /// @brief constructor
    pinch_detector ()
    {
        sm.init (state::start);
        //      state                   event           action                          next state
        //      -----                   -----           ------                          ----------
        sm.add (state::start,           event::open,    &pinch_detector::start_timer1,  state::open_waiting);
        sm.add (state::open_waiting,    event::open,    &pinch_detector::do_nothing,    state::open_waiting);
        sm.add (state::open_waiting,    event::timer1,  &pinch_detector::do_nothing,    state::open);
        sm.add (state::open,            event::open,    &pinch_detector::do_nothing,    state::open);
        sm.add (state::open,            event::zero,    &pinch_detector::do_nothing,    state::zero);
        sm.add (state::open,            event::closed,  &pinch_detector::do_nothing,    state::closed);
        sm.add (state::zero,            event::zero,    &pinch_detector::do_nothing,    state::zero);
        sm.add (state::zero,            event::closed,  &pinch_detector::do_nothing,    state::closed);
        sm.add (state::closed,          event::closed,  &pinch_detector::do_nothing,    state::closed);
        sm.add (state::closed,          event::open,    &pinch_detector::do_nothing,    state::accepted);
    }
    // public functions
    bool is_set () const
    {
        return sm.get_state () == state::accepted;
    }
    void reset ()
    {
        sm.init (state::start);
        timer1.reset ();
        timer2.reset ();
    }
    bool maybe () const
    {
        switch (sm.get_state ())
        {
            default:
            assert (0); // logic error
            case state::start:
            case state::open_waiting:
            case state::open:
            return false;
            case state::zero:
            case state::closed:
            case state::accepted:
            return true;
        }
    }
    void update (const uint64_t ts, const hand_sample &s)
    {
        if (timer1.is_set () && !timer1.is_on (ts))
        {
            timer1.reset ();
            sm.record (event::timer1, *this, ts);
        }
        else if (timer2.is_set () && !timer2.is_on (ts))
        {
            timer2.reset ();
            sm.record (event::timer2, *this, ts);
        }
        switch (s.size ())
        {
            default:
            {
                sm.record (event::invalid, *this, ts);
                break;
            }
            case 0:
            case 1:
            {
                // d = 0.0
                sm.record (event::zero, *this, ts);
                break;
            }
            case 2:
            {
                double d = s[0].position.distanceTo (s[1].position);
                //dd.update (ts, d);
                if (d > OPEN_MIN)
                {
                    //if (dd.current () > dd.last ())
                    sm.record (event::open, *this, ts);
                }
                else // d < OPEN_MIN
                {
                    if (d > std::numeric_limits<float>::min ())
                        sm.record (event::closed, *this, ts);
                    else
                        sm.record (event::zero, *this, ts);
                }
                break;
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
            return true;
        std::vector<int> s = k.key_states ();
        // was alt pressed?
        if (s[2] || s[3])
            return true;
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
        // reset it
        pd.reset ();
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
