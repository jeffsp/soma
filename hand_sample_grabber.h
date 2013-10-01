/// @file hand_sample_grabber.h
/// @brief hand_sample_grabber class
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-23

#ifndef HAND_SAMPLE_GRABBBER_H
#define HAND_SAMPLE_GRABBBER_H

#include "hand_sample.h"
#include <unistd.h>

namespace soma
{

typedef std::vector<uint64_t> timestamps;
typedef std::vector<hand_sample> hand_samples;

class hand_sample_grabber : public Leap::Listener
{
    private:
    bool on;
    uint64_t max_time;
    timestamps ts;
    hand_samples hs;
    public:
    hand_sample_grabber ()
        : on (false)
        , max_time (0)
    {
    }
    virtual void onFrame (const Leap::Controller& c)
    {
        // don't do anything if it's off
        if (!on)
            return;
        // get the frame
        Leap::Frame f = c.frame ();
        // save the frame info
        hs.push_back (hand_sample (f.pointables ()));
        ts.push_back (f.timestamp ());
        if (ts.back () - ts.front () >= max_time)
            on = false;
    }
    const hand_samples &get_hand_samples ()
    {
        return hs;
    }
    const uint64_t current_timestamp () const
    {
        if (ts.empty ())
            return 0;
        return ts.back ();
    }
    const timestamps &get_timestamps ()
    {
        return ts;
    }
    void grab (uint64_t usec)
    {
        assert (!on);
        // start with no frames
        hs.clear ();
        ts.clear ();
        // set the time
        max_time = usec;
        // start getting frames
        on = true;
        // wait until it is done
        while (on)
            usleep (1000);
    }
};

}

#endif
