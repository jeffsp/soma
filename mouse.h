/// @file mouse.h
/// @brief mouse control
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-04

#ifndef MOUSE_H
#define MOUSE_H

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#elif defined(__APPLE__) && defined(__MACH__)
#elif defined(_MSC_VER)
#else
#error("unknown OS")
#endif

#include <stdexcept>

namespace soma
{

class mouse
{
    private:
    Display *d;
    public:
    mouse ()
        : d (XOpenDisplay (0))
    {
        if (!d)
            throw std::runtime_error ("Could not open X display");
    }
    ~mouse ()
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
    void set (int x, int y)
    {
        const int screen_number = 0;
        Window dest = RootWindow (d, screen_number);
        XWarpPointer (d, None, dest, 0, 0, 0, 0, x, y);
        XFlush (d);
    }
    void center ()
    {
        const int screen_number = 0;
        Screen *s = ScreenOfDisplay (d, screen_number);
        if (!s)
            throw std::runtime_error ("could not determine screen of display");
        int w = WidthOfScreen (s);
        int h = HeightOfScreen (s);
        std::clog << "screen dimensions " << w << "X" << h << std::endl;
        std::clog << "moving to " << w/2 << "," << h/2 << std::endl;
        if (w != 0 && h != 0)
            set (w / 2, h / 2);
    }
};

}

#endif
