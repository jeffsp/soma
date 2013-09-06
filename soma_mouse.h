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

#include "audio.h"
#include "mouse.h"
#include "options.h"
#include "soma.h"
#include "stats.h"
#include "utility.h"

namespace soma
{

const uint64_t FINGER_COUNTER_WINDOW_DURATION = 200000;
const float FINGER_COUNTER_WINDOW_FULLNESS = 0.85f;
const float FINGER_COUNTER_CERTAINTY = 0.8f;

class finger_counter
{
    private:
    sliding_time_window<unsigned> w;
    unsigned current_count;
    public:
    finger_counter ()
        : w (FINGER_COUNTER_WINDOW_DURATION)
        , current_count (~0)
    {
    }
    void update (uint64_t ts, const unsigned count)
    {
        w.add_sample (ts, count);
        // if it's mostly full
        if (w.fullness (ts) > FINGER_COUNTER_WINDOW_FULLNESS)
        {
            // get count
            auto s = w.get_samples ();
            assert (s.size () > 0);
            unsigned count = mode (s);
            float total = 0.0f;
            for (auto i : s)
                if (i == count)
                    ++total;
            float certainty = total / s.size ();
            // don't change unless certainty is high
            if (certainty < FINGER_COUNTER_CERTAINTY)
                return;
            current_count = count;
        }
    }
    unsigned get_count () const
    {
        return current_count;
    }
};

class points_tracker
{
    private:
    points current_points;
    public:
    void update (const unsigned count, const points &p)
    {
        // simply filter out points samples that don't have the correct current size
        if (count == p.size ())
            current_points = p;
    }
    const points &get_points () const
    {
        return current_points;
    }
};

enum class input_mode { zero, point, click, scroll, center };

std::string to_string (const input_mode m)
{
    switch (m)
    {
        default: assert (0); return std::string ("unknown");
        case input_mode::zero: return std::string ("zero");
        case input_mode::point: return std::string ("point");
        case input_mode::click: return std::string ("click");
        case input_mode::scroll: return std::string ("scroll");
        case input_mode::center: return std::string ("center");
    }
}

class mode_switcher
{
    private:
    input_mode current;
    public:
    mode_switcher ()
        : current (input_mode::zero)
    {
    }
    void update (const points &p)
    {
        switch (p.size ())
        {
            default:
            current = input_mode::zero;
            break;
            case 1:
            current = input_mode::point;
            break;
            case 2:
            current = input_mode::scroll;
            break;
            case 5:
            current = input_mode::center;
            break;
        }
    }
    input_mode get_mode () const
    {
        return current;
    }
};

void beep (const audio &au, const input_mode &m)
{
    const int D = 200; // millisecs
    switch (m)
    {
        default: assert (0); break;
        case input_mode::zero: au.play (98, D); break;
        case input_mode::point: au.play (262, D); break;
        case input_mode::click: au.play (392, D); break;
        case input_mode::scroll: au.play (523, D); break;
        case input_mode::center:
        au.play (262, D);
        au.play (1, D);
        au.play (262, D);
        au.play (1, D);
        au.play (262, D);
        break;
    }
}

class soma_mouse : public Leap::Listener
{
    private:
    bool done;
    const options &opts;
    frame_counter frc;
    finger_counter fic;
    points_tracker pt;
    mode_switcher ms;
    audio au;
    public:
    soma_mouse (const options &opts)
        : done (false)
        , opts (opts)
    {
    }
    ~soma_mouse ()
    {
        std::clog << frc.fps () << "fps" << std::endl;
    }
    bool is_done () const
    {
        return done;
    }
    virtual void onInit (const Leap::Controller&)
    {
        std::clog << "onInit()" << std::endl;
    }
    virtual void onConnect (const Leap::Controller&)
    {
        std::clog << "onConnect()" << std::endl;
    }
    virtual void onDisconnect (const Leap::Controller&)
    {
        std::clog << "onDisconnect()" << std::endl;
    }
    virtual void onFrame(const Leap::Controller& c)
    {
        if (done)
            return;
        const Leap::Frame &f = c.frame ();
        frc.update (f.timestamp ());
        const points &p = get_points (f.pointables ());
        fic.update (f.timestamp (), p.size ());
        static unsigned last_count = ~0;
        unsigned this_count = fic.get_count ();
        if (this_count != last_count)
        {
            last_count = this_count;
            std::clog << "count " << this_count << std::endl;
        }
        pt.update (fic.get_count (), p);
        static points last_points;
        const points &this_points = pt.get_points ();
        if (this_points != last_points)
        {
            last_points = this_points;
            std::clog << "points " << this_points << std::endl;
        }
        ms.update (this_points);
        static input_mode last_mode = input_mode::zero;
        input_mode this_mode = ms.get_mode ();
        if (this_mode != last_mode)
        {
            last_mode = this_mode;
            std::clog << "mode " << to_string (this_mode) << std::endl;
            if (opts.get_sound ())
                beep (au, this_mode);
        }
    }
};

}

#endif
