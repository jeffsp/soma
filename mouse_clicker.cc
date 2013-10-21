/// @file mouse_clicker.cc
/// @brief detect mouse clicks
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-17

#include "mouse.h"
#include "mouse_clicker.h"
#include <stdexcept>

using namespace std;
using namespace soma;
const string usage = "usage: mouse_clicker";

/// @brief grab frames and check clicks
class grabber : public Leap::Listener
{
    private:
    mouse m;
    mouse_clicker mc;
    public:
    grabber ()
        : mc (m)
    {
    }
    virtual void onFrame (const Leap::Controller& c)
    {
        // get the frame
        Leap::Frame f = c.frame ();
        uint64_t ts = f.timestamp ();
        // get the sample
        hand_sample hs (f.pointables ());
        // update the clicker
        mc.update (ts, hs);
        // check states
        if (mc.did_pinch (ts))
            std::clog << "pinched" << std::endl;
        if (mc.did_left_click (ts))
            std::clog << "left click" << std::endl;
        if (mc.did_right_click (ts))
            std::clog << "right click" << std::endl;
        /*
        if (mc.maybe_pinched (ts))
            std::clog << "maybe pinched" << std::endl;
        if (mc.maybe_left_clicked (ts))
            std::clog << "maybe left clicked" << std::endl;
        if (mc.maybe_right_clicked (ts))
            std::clog << "maybe right clicked" << std::endl;
        */
    }
};

int main (int argc, char **argv)
{
    try
    {
        if (argc != 1)
            throw runtime_error (usage);

        clog << "Press CTRL-C to exit" << endl;

        grabber g;
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
