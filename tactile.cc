/// @file tactile.cc
/// @brief tactile leap interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-08-25

#include "tactile.h"

using namespace std;
using namespace tactile;
const string usage = "usage: tactile";

int main (int argc, char **argv)
{
    try
    {
        if (argc != 1)
            throw runtime_error (usage);

        listener l;
        Leap::Controller c (l);

        // set to receive frames in the background
        c.setPolicyFlags (Leap::Controller::POLICY_BACKGROUND_FRAMES);

        clog << "press control-C to quit" << endl;

        while (!l.is_done ())
        {
        }

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
