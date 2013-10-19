/// @file finger_id_tracker.cc
/// @brief track finger ids
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-27

#include <stdexcept>
#include "finger_id_tracker.h"
#include "hand_sample.h"
#include "Leap.h"
#include <unistd.h>

using namespace std;
using namespace soma;
const string usage = "usage: finger_id_tracker";

class grabber : public Leap::Listener
{
    private:
    finger_id_tracker fit;
    public:
    grabber (uint64_t duration)
        : fit (duration)
    {
    }
    virtual void onFrame (const Leap::Controller& c)
    {
        // get the frame
        Leap::Frame f = c.frame ();
        uint64_t ts = f.timestamp ();
        // get the sample
        hand_sample hs (f.pointables ());
        finger_ids ids;
        for (auto j : hs)
            ids.push_back (j.id);
        // add it to the tracker
        fit.add (ts, ids);
        // if it's changed, print the result
        for (size_t nfingers = 0; nfingers < 10; ++nfingers)
        {
            if (fit.has_changed (nfingers))
            {
                for (auto i : fit.get_ids (nfingers))
                    clog << ' ' << i;
                clog << endl;
            }
        }
    }
};

int main (int argc, char **argv)
{
    try
    {
        if (argc != 1)
            throw runtime_error (usage);

        clog << "Press CTRL-C to exit" << endl;

        static const uint64_t WINDOW_DURATION = 1000000;
        grabber g (WINDOW_DURATION);
        Leap::Controller c (g);

        // receive frames even when you don't have focus
        c.setPolicyFlags (Leap::Controller::POLICY_BACKGROUND_FRAMES);
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
