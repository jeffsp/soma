/// @file test2.cc
/// @brief test mouse interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-010-22

#include "soma.h"
#include <unistd.h>

using namespace std;
using namespace soma;
const string usage = "usage: test2";

enum class point_mode { fast, slow };

double mm_to_pixels (const double mm)
{
    return mm * 3.7795;
}

double pixels_to_mm (const double pixels)
{
    return pixels / 3.7795;
}

class test2 : public Leap::Listener
{
    private:
    static const uint64_t SW_DURATION = 50000;
    sliding_window<double> swx;
    sliding_window<double> swy;
    running_mean smooth_x;
    running_mean smooth_y;
    static const uint64_t FINGER_COUNTER_WINDOW_DURATION = 200000;
    bool done;
    finger_counter fc;
    mouse m;
    point_delta<vec3> dd;
    void update (uint64_t ts, const hand_sample &s)
    {
        // add it to the counter
        fc.add (ts, s.size ());
        if (s.size () == 2 || s.size () == 1)
        {
            const double MIND = 40;
            const double MAXD = 100;
            vec3 p = s[0].position;
            double d = MIND;
            if (s.size () == 2)
            {
                p = s[1].position;
                d = s[0].position.distanceTo (s[1].position);
            }
            const double x = p.x;
            const double y = p.y;
            swx.add (ts, x, smooth_x);
            swy.add (ts, y, smooth_y);
            const double sx = smooth_x.get_mean ();
            const double sy = smooth_y.get_mean ();
            // get index pointer
            dd.update (ts, vec3 (sx, sy, 0));
            const double dx = dd.current ().x - dd.last ().x;
            const double dy = dd.last ().y - dd.current ().y;
            const double MING = 0.5;
            const double MAXG = 5.0;
            double gain = (d - MIND) / MAXD;
            gain = gain < 0.0 ? 0.0 : gain;
            gain = gain > 1.0 ? 1.0 : gain;
            gain = gain * (MAXG - MING) + MING;
            // make sure time delta is a reasonable value
            if (dd.dt () == 0 || dd.dt () > 500000)
                return;
            // make it independent of framerate
            const double fr = 1000000.0 / dd.dt ();
            const double mx = dx * 100 / fr;
            const double my = dy * 100 / fr;
            // convert to pixels
            const double px =  mm_to_pixels (mx);
            const double py =  mm_to_pixels (my);
            std::clog << fr << '\t' << px << '\t' << py << std::endl;
            m.move (gain * px, gain * py);
        }
    }
    public:
    test2 ()
        : swx (SW_DURATION)
        , swy (SW_DURATION)
        , done (false)
        , fc (FINGER_COUNTER_WINDOW_DURATION)
    {
    }
    ~test2 ()
    {
    }
    bool is_done () const
    {
        return done;
    }
    virtual void onFrame (const Leap::Controller& c)
    {
        if (done)
            return;
        // get the frame
        Leap::Frame f = c.frame ();
        uint64_t ts = f.timestamp ();
        // get the sample
        hand_sample s (f.pointables ());
        // quit?
        if (s.size () > 6)
        {
            done = true;
            return;
        }
        hand_sample tmp (s);
        sort (tmp.begin (), tmp.end (), sort_left_to_right);
        // update the mouse
        update (ts, tmp);
    }
};
int main (int argc, char **argv)
{
    try
    {
        if (argc != 1)
            throw runtime_error (usage);

        test2 t;
        Leap::Controller c (t);

        clog << "7 fingers = quit" << endl;

        // receive frames even when you don't have focus
        c.setPolicyFlags (Leap::Controller::POLICY_BACKGROUND_FRAMES);

        // loop
        while (!t.is_done ())
            usleep (5000);

        clog << "done" << endl;

        usleep (1000000);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
