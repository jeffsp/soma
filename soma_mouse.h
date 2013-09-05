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
#include "utility.h"
#include "Leap.h"

namespace soma
{

class mode_switcher
{
    private:
    enum class mode : int { zero, point, click, scroll, center };
    mode current;
    mode last;
    public:
    mode_switcher ()
        : current (mode::zero)
        , last (mode::zero)
    {
    }
    mode get_mode (const finger_tips &ft) const
    {
        return current;
    }
};

class soma_mouse : public Leap::Listener
{
    private:
    bool done;
    frame_counter fc;
    finger_tracker ft;
    //bool sound_flag;
    audio au;
    public:
    soma_mouse ()
        : done (false)
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
        fc.update (f.timestamp ());
        /*
         * ft.update (f.timestamp (), f.pointables ());
         */
        if (ft.is_changed ())
            std::clog << " fingers " << ft.count () << std::endl;
        if (ft.count () == 5)
            done = true;
        //ip.update (f.timestamp (), f.pointables ());
        //p1d.update (f.timestamp (), f.pointables ());
    }
};

}

#endif
