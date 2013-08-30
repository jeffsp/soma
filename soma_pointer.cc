/// @file soma_pointer.cc
/// @brief soma pointer leap interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-08-30

#include "soma.h"
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

using namespace std;
using namespace soma;
const string usage = "usage: soma_pointer";

class X
{
    private:
    Display *d;
    public:
    X ()
        : d (XOpenDisplay (0))
    {
        if (!d)
            throw runtime_error ("Could not open X display");
    }
    ~X ()
    {
        XCloseDisplay (d);
    }
    void Click (int button, Bool down)
    {
        XTestFakeButtonEvent (d, button, down, CurrentTime);
        XFlush (d);
    }
    void Move (int x, int y)
    {
        XWarpPointer (d, None, None, 0, 0, 0, 0, x, y);
        XFlush (d);
    }
};

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

        X x;

        clog << "press control-C to quit" << endl;

        while (!l.is_done ())
        {
            x.Click (0, true);
            x.Click (0, false);
            x.Move (100, 100);
        }

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
