/// @file measure_samples.cc
/// @brief get samples and measure stats
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-06

#include "soma.h"
#include "stats.h"

using namespace std;
using namespace soma;
const string usage = "usage: measure_samples";

template<typename W>
void stats (const W &w, size_t index)
{
    auto s = w.get_samples ();
    vector<float> x;
    vector<float> y;
    vector<float> z;
    vector<float> a;
    for (size_t i = 0; i + 1 < s.size (); ++i)
    {
        if (s[i].size () > index)
        {
            float fx = s[i][index].x;
            float fy = s[i][index].y;
            float fz = s[i][index].z;
            x.push_back (fx);
            y.push_back (fy);
            z.push_back (fz);
            a.push_back (sqrt (fx * fx + fy * fy + fz * fz));
        }
    }
    if (!x.empty ())
        clog
            << ' ' << average (x)
            << ' ' << average (y)
            << ' ' << average (z)
            << ' ' << average (a)
            << ' ' << sqrt (variance (x))
            << ' ' << sqrt (variance (y))
            << ' ' << sqrt (variance (z))
            << ' ' << sqrt (variance (a))
            << endl;
    else
        clog << endl;
}

template<typename W>
void stats (const W &w)
{
    auto s = w.get_samples ();
    vector<float> d;
    vector<float> xy;
    vector<float> yz;
    vector<float> zx;
    for (size_t i = 0; i + 1 < s.size (); ++i)
    {
        if (s[i].size () == 2)
        {
            float dx = s[i][0].x - s[i][1].x;
            float dy = s[i][0].y - s[i][1].y;
            float dz = s[i][0].z - s[i][1].z;
            d.push_back (sqrt (dx * dx + dy * dy + dz * dz));
            xy.push_back (atan2 (dx, dy) * 180 / M_PI);
            yz.push_back (atan2 (dy, dz) * 180 / M_PI);
            zx.push_back (atan2 (dz, dx) * 180 / M_PI);
        }
    }
    if (!d.empty ())
        clog
            << ' ' << average (d)
            << ' ' << average (xy)
            << ' ' << average (yz)
            << ' ' << average (zx)
            << ' ' << sqrt (variance (d))
            << ' ' << sqrt (variance (xy))
            << ' ' << sqrt (variance (yz))
            << ' ' << sqrt (variance (zx))
            << endl;
    else
        clog << endl;
}

class soma_measure : public Leap::Listener
{
    private:
    bool done;
    sliding_time_window<vec3s> pw;
    sliding_time_window<vec3s> vw;
    sliding_time_window<vec3s> dw;
    public:
    soma_measure ()
        : done (false)
        , pw (3000000)
        , vw (3000000)
        , dw (3000000)
    {
    }
    bool is_done () const
    {
        return done;
    }
    virtual void onFrame(const Leap::Controller& c)
    {
        if (done)
            return;
        const Leap::Frame &f = c.frame ();
        const vec3s &p = get_positions (f.pointables ());
        const vec3s &v = get_velocities (f.pointables ());
        const vec3s &d = get_directions (f.pointables ());
        if (p.size () == 6)
        {
            done = true;
        }
        else
        {
            pw.add_sample (f.timestamp (), p);
            vw.add_sample (f.timestamp (), v);
            dw.add_sample (f.timestamp (), d);
            if (pw.fullness (f.timestamp ()) > 0.9)
            {
                clog << "position ";
                stats (pw, 0);
                clog << "position diffs ";
                stats (pw);
                pw.clear ();
            }
            if (vw.fullness (f.timestamp ()) > 0.9)
            {
                clog << "velocity ";
                stats (vw, 0);
                vw.clear ();
            }
            if (dw.fullness (f.timestamp ()) > 0.9)
            {
                clog << "direction0";
                stats (dw, 0);
                clog << "direction1";
                stats (dw, 1);
                dw.clear ();
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

        soma_measure sd;
        Leap::Controller c (sd);

        clog << "6 fingers = quit" << endl;
        clog << "measuring..." << endl;

        while (!sd.is_done ())
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
