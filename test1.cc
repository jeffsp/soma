/// @file test1.cc
/// @brief test mouse interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-010-22

#include "soma.h"
#include <unistd.h>

using namespace std;
using namespace soma;
const string usage = "usage: test1";

enum class point_mode { fast, slow };

class test1 : public Leap::Listener
{
    private:
    static const uint64_t SW_DURATION = 100000;
    static const uint64_t SWS_DURATION = 200000;
    sliding_window<double> swx;
    sliding_window<double> swy;
    sliding_window<double> swx2;
    sliding_window<double> swy2;
    sliding_window<double> swdx;
    sliding_window<double> swdy;
    running_mean smooth_x;
    running_mean smooth_y;
    running_mean smooth_x2;
    running_mean smooth_y2;
    running_mean smooth_dx;
    running_mean smooth_dy;
    static const uint64_t FINGER_COUNTER_WINDOW_DURATION = 200000;
    bool done;
    finger_counter fc;
    mouse m;
    point_mode pm;
    point_delta<vec3> dd;
    double mapx (const double theta) const
    {
        double t = (theta - 80) / 50;
        //std::clog << "theta " << theta << "\tx " << t << std::endl;
        return t * m.width ();
    }
    double mapy (const double theta) const
    {
        double t = 1 - (theta - 60) / 50;
        //std::clog << "theta " << theta << "\ty " << t << std::endl;
        return t * m.height ();
    }
    void update (uint64_t ts, const hand_sample &s)
    {
        // add it to the counter
        fc.add (ts, s.size ());
        // if it's changed, print the result
        //if (fc.has_changed ())
        //    clog << fc.get_count () << endl;
        if (s.size () == 2)
        {
            // get index pointer
            dd.update (ts, s[1].position);
            // get slopes
            vec3 p0 = s[0].position;
            vec3 p1 = s[1].position;
            const double fdx = p1.x - p0.x;
            const double fdy = p1.y - p0.y;
            const double fdz = p1.z - p0.z;
            const double zx_slope = atan2 (fdz, fdx) * 180 / M_PI;
            const double zy_slope = atan2 (fdz, fdy) * 180 / M_PI;
            //std::clog << zy_slope << std::endl;
            double x = mapx (zx_slope);
            double y = mapy (zy_slope);
            swx.add (ts, x, smooth_x);
            swy.add (ts, y, smooth_y);
            swx2.add (ts, x * x, smooth_x2);
            swy2.add (ts, y * y, smooth_y2);
            const double sx = smooth_x.get_mean ();
            const double sy = smooth_y.get_mean ();
            const double vx = smooth_x2.get_mean () - sx * sx;
            const double vy = smooth_y2.get_mean () - sy * sy;
            const double dx = dd.current ().x - dd.last ().x;
            const double dy = dd.last ().y - dd.current ().y;
            swdx.add (ts, dx, smooth_dx);
            swdy.add (ts, dy, smooth_dy);
            std::clog << dx << '\t' << dy << std::endl;
            if (dx < 0.1 && dy < 0.1)
                pm = point_mode::slow;
            else
                pm = point_mode::fast;
            switch (pm)
            {

                default: assert (0); // logic error
                case point_mode::slow:
                {
                    m.move (5 * smooth_dx.get_mean (), 5 * smooth_dy.get_mean ());
                    break;
                }
                case point_mode::fast:
                {
                    m.set (sx, sy);
                    break;
                }
            }
        }
    }
    public:
    test1 ()
        : swx (SW_DURATION)
        , swy (SW_DURATION)
        , swx2 (SW_DURATION)
        , swy2 (SW_DURATION)
        , swdx (SWS_DURATION)
        , swdy (SWS_DURATION)
        , done (false)
        , fc (FINGER_COUNTER_WINDOW_DURATION)
        , pm (point_mode::slow)
    {
    }
    ~test1 ()
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
        sort (tmp.begin (), tmp.end (), sort_back_to_front);
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

        test1 t;
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
