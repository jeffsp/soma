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
#include "time_guard.h"
#include <fstream>

namespace soma
{

class mouse_clicker
{
    private:
    static const uint64_t CLICK_GUARD_DURATION = 300000;
    time_guard can_click;
    keyboard k;
    mouse &m;
    public:
    mouse_clicker (mouse &m)
        : m (m)
    {
    }
    void update (const uint64_t ts, const hand_sample &h)
    {
    }
    bool did_pinch (uint64_t ts) const
    {
        if (can_click.guarded (ts))
            return false;
        return false;
    }
    bool did_left_click (uint64_t ts) const
    {
        if (can_click.guarded (ts))
            return false;
        std::vector<int> s = k.key_states ();
        // was shift pressed?
        if (s[0] || s[1])
            return true;
        return false;
    }
    bool did_right_click (uint64_t ts) const
    {
        if (can_click.guarded (ts))
            return false;
        std::vector<int> s = k.key_states ();
        // was ctrl pressed?
        if (s[4] || s[5])
            return true;
        return false;
    }
    bool maybe_pinched (uint64_t ts) const
    {
        return false;
    }
    bool maybe_left_clicked (uint64_t ts) const
    {
        return false;
    }
    bool maybe_right_clicked (uint64_t ts) const
    {
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
