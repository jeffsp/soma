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
    Window root;
    int w;
    int h;
    public:
    mouse ()
        : d (XOpenDisplay (0))
    {
        if (!d)
            throw std::runtime_error ("Could not open X display");
        const int screen_number = 0;
        root = RootWindow (d, screen_number);
        Screen *s = ScreenOfDisplay (d, screen_number);
        if (!s)
            throw std::runtime_error ("could not determine screen of display");
        w = WidthOfScreen (s);
        h = HeightOfScreen (s);
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
        XWarpPointer (d, None, root, 0, 0, 0, 0, x, y);
        XFlush (d);
    }
    int width () const
    {
        return w;
    }
    int height () const
    {
        return h;
    }
};

}

#endif
