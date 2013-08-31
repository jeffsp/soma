/// @file soma_pointer.cc
/// @brief soma pointer leap interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-08-30

#include "soma.h"

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <ApplicationServices/ApplicationServices.h>
#elif defined(_MSC_VER)
#else
#error("unknown OS")
#endif

using namespace std;
using namespace soma;
using namespace Leap;
const string usage = "usage: soma_pointer";

#ifdef __linux__
class Mouse
{
    private:
    Display *d;
    public:
    Mouse ()
        : d (XOpenDisplay (0))
    {
        if (!d)
            throw runtime_error ("Could not open X display");
    }
    ~Mouse ()
    {
        XCloseDisplay (d);
    }
    void click (int button, Bool down)
    {
        XTestFakeButtonEvent (d, button, down, CurrentTime);
        XFlush (d);
    }
    void move (int x, int y)
    {
        XWarpPointer (d, None, None, 0, 0, 0, 0, x, y);
        XFlush (d);
    }
};
#elif defined(__APPLE__) && defined(__MACH__)
class Mouse
{
    private:
    public:
    void click (int button, bool down)
    {
    }
    void move (int x, int y)
    {
        CGEventPost (kCGHIDEventTap,
            CGEventCreateMouseEvent
                (NULL,
                 kCGEventMouseMoved,
                 CGPointMake(x, y),
                 kCGMouseButtonLeft));
    }
};
#elif defined(_MSC_VER)
#else
#error("unknown OS")
#endif

class finger_pointer : public Listener
{
    private:
    bool done;
    frame_counter frc;
    finger_counter fic;
    sliding_time_window<Vector> pos;
    Mouse mouse;
    template<typename T>
    void move (const T &s)
    {
        if (s.size () < 2)
            return;
        auto a = s[s.size () - 1];
        auto b = s[s.size () - 2];
        auto x = a.x - b.x;
        auto y = a.y - b.y;
        auto z = a.z - b.z;
        if (z > x && z > y && z > 1)
            std::clog << "Click" << std::endl;
        else
            mouse.move (-x, y);
    }

    public:
    finger_pointer (uint64_t window_duration)
        : done (false)
        , fic (window_duration)
        , pos (window_duration)
    {
    }
    bool is_done () const
    {
        return done;
    }
    virtual void onInit (const Controller&)
    {
        clog << "onInit()" << endl;
    }
    virtual void onConnect (const Controller&)
    {
        clog << "onConnect()" << endl;
    }
    virtual void onDisconnect (const Controller&)
    {
        clog << "onDisconnect()" << endl;
    }
    virtual void onFrame(const Controller& c)
    {
        const Frame &f = c.frame ();
        frc.update (f.timestamp ());
        fic.update (f.timestamp (), f.pointables ());
        if (fic.is_changed ())
            clog << " fingers " << fic.count () << endl;
        if (fic.count () == 5)
            done = true;
        else if (fic.count () == 1)
        {
            auto p = f.pointables ();
            if (p.count () == 1)
            {
                pos.update (f.timestamp ());
                pos.add_sample (f.timestamp (), p[0].tipPosition ());
                if (pos.full (85, f.timestamp ()))
                {
                    auto s = pos.get_samples ();
                    auto d = distances (s);
                    if (average (d) > 1.0f)
                        move (s);
                }
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

        finger_pointer fp (200000);
        Controller c (fp);

        // set to receive frames in the background
        c.setPolicyFlags (Controller::POLICY_BACKGROUND_FRAMES);

        clog << "press control-C to quit" << endl;

        while (!fp.is_done ())
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
