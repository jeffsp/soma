/// @file tactile.h
/// @brief tactile software for Leap controller
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-08-26

#ifndef TACTILE_H
#define TACTILE_H

#include "Leap.h"
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace tactile
{

typedef std::chrono::system_clock clock;

std::string to_string (const clock::time_point &tp)
{
    std::time_t t = clock::to_time_t (tp);
    std::string ts = std::ctime (&t);
    ts.resize (ts.size () - 1);
    return ts;
}

class Listener : public Leap::Listener
{
    private:
    public:
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
        std::clog << "onFrame()" << std::endl;
        Leap::Frame f = c.frame();
        // assume timestamp has the same epoch as 'clock'
        std::chrono::microseconds d1 (f.timestamp ());
        std::chrono::time_point<clock> ft (d1);
        std::clog << "frame timestamp: " << to_string (ft) << std::endl;
        std::chrono::time_point<clock> t2 = clock::now ();
        std::clog << "current timestamp: " << t2.time_since_epoch ().count () << " " << to_string (t2) << std::endl;
    }
};

}

#endif
