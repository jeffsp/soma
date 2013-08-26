/// @file tactile.h
/// @brief tactile software for Leap controller
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-08-26

#ifndef TACTILE_H
#define TACTILE_H

#include "Leap.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace tactile
{

class Listener : public Leap::Listener
{
    private:
    bool done;
    uint64_t frames;
    uint64_t first_timestamp;
    uint64_t last_timestamp;
    public:
    Listener ()
        : done (false)
        , frames (0)
        , first_timestamp (0)
        , last_timestamp (0)
    {
    }
    ~Listener ()
    {
        double secs = (last_timestamp - first_timestamp) / 1000000.0;
        if (secs != 0.0)
            std::clog << frames / secs << "fps" << std::endl;
        else
            std::clog << "nan fps" << std::endl;
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
        const Leap::Frame &f = c.frame ();
        uint64_t ts = f.timestamp ();
        if (frames == 0)
            first_timestamp = ts;
        else
            last_timestamp = ts;
        ++frames;
        const Leap::HandList &hands = f.hands ();
        for (int i = 0; i < hands.count (); ++i)
        {
            const Leap::Hand &h = hands[i];
            std::clog << "hand " << i;
            if (!h.isValid ())
            {
                std::clog << " invalid" << std::endl;
            }
            else
            {
                std::clog << " fingers " << h.fingers ().count () << std::endl;
                if (h.fingers ().count () == 5)
                    done = true;
            }
        }
    }
};

}

#endif
