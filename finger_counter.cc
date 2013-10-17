/// @file finger_counter.cc
/// @brief count fingers
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-27

#include "finger_counter.h"
#include "hand_sample.h"
#include "Leap.h"
#include <stdexcept>

using namespace std;
using namespace soma;
const string usage = "usage: finger_counter";

/// @brief grab frames and count fingers
class grabber : public Leap::Listener
{
    private:
    finger_counter fc;
    public:
    grabber (uint64_t duration)
        : fc (duration)
    {
    }
    virtual void onFrame (const Leap::Controller& c)
    {
        // get the frame
        Leap::Frame f = c.frame ();
        uint64_t ts = f.timestamp ();
        // get the sample
        hand_sample hs (f.pointables ());
        // add it to the counter
        fc.add (ts, hs.size ());
        // if it's changed, print the result
        if (fc.has_changed ())
            clog << fc.get_count () << endl;
    }
};

int main (int argc, char **argv)
{
    try
    {
        if (argc != 1)
            throw runtime_error (usage);

        clog << "Press CTRL-C to exit" << endl;

        static const uint64_t WINDOW_DURATION = 200000;
        grabber g (WINDOW_DURATION);
        Leap::Controller c (g);

        // receive frames even when you don't have focus
        c.setPolicyFlags (Leap::Controller::POLICY_BACKGROUND_FRAMES);

        // loop
        while (1)
            usleep (5000);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
