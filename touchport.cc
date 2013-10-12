/// @file touchport.cc
/// @brief instead of a viewport, it's a touchport
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-11

#include <stdexcept>
#include "soma.h"

using namespace std;
using namespace soma;
const string usage = "usage: touchport > touchport.txt";

vec3 find (const vec3 &c, const vector<vec3> &p, bool xltz, bool yltz)
{
    vec3 m;
    double d = 0.0;
    for (auto i : p)
    {
        if (((i.x < c.x) == xltz) && ((i.y < c.y) == yltz))
        {
            double t = i.distanceTo (c);
            if (t > d)
            {
                d = t;
                m = i;
            }
        }
    }
    return m;
}

class touchport : public Leap::Listener
{
    private:
    bool done;
    vector<vec3> points;
    vec3 tl;
    vec3 tr;
    vec3 bl;
    vec3 br;
    public:
    touchport (uint64_t duration)
        : done (false)
    {
    }
    void print (ostream &s)
    {
        double cx = 0.0;
        double cy = 0.0;
        double cz = 0.0;
        for (auto i : points)
        {
            cx += i.x;
            cy += i.y;
            cz += i.z;
        }
        vec3 c (cx / points.size (), cy / points.size (), cz / points.size ());
        tl = find (c, points, 1, 0);
        tr = find (c, points, 0, 0);
        bl = find (c, points, 1, 1);
        br = find (c, points, 0, 1);
        s << tl << endl;
        s << tr << endl;
        s << bl << endl;
        s << br << endl;
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
        // get the sample
        hand_sample hs (f.pointables ());
        // 5 fingers == exit
        if (hs.size () == 5)
            done = true;
        // pointer
        if (hs.size () == 1)
        {
            vec3 p = hs[0].position;
            points.push_back (p);
        }
    }
};

int main (int argc, char **argv)
{
    try
    {
        if (argc != 1)
            throw runtime_error (usage);

        clog << "5 fingers = exit" << endl;

        static const uint64_t WINDOW_DURATION = 200000;
        touchport tp (WINDOW_DURATION);
        Leap::Controller c (tp);

        // receive frames even when you don't have focus
        c.setPolicyFlags (Leap::Controller::POLICY_BACKGROUND_FRAMES);

        // loop
        while (!tp.is_done ())
            usleep (5000);

        tp.print (cout);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
