/// @file hand_shape_classifier.cc
/// @brief classify hand shapes
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-20

#include "hand_shape_classifier.h"
#include "Leap.h"
#include <stdexcept>

using namespace std;
using namespace soma;
const string usage = "usage: hand_shape_classifier";

class grabber : public Leap::Listener
{
    private:
    hand_shape_classifier hsc;
    public:
    grabber (uint64_t duration)
        : hsc (duration)
    {
    }
    virtual void onFrame (const Leap::Controller& c)
    {
        // get the frame
        Leap::Frame f = c.frame ();
        uint64_t ts = f.timestamp ();
        // get the sample
        hand_sample s (f.pointables ());
        // add it to the classifier
        hsc.add (ts, s);
        // if it's changed, print the result
        if (hsc.has_changed ())
            clog << to_string (hsc.get_shape ()) << endl;
    }
};

int main (int argc, char **argv)
{
    try
    {
        if (argc != 1)
            throw runtime_error (usage);

        clog << "Press CTRL-C to exit" << endl;

        static const uint64_t WINDOW_DURATION = 400000;
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
