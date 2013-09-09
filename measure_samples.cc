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

template<typename T>
float distance (const T &a, const T &b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return sqrt (dx * dx + dy * dy + dz * dz);
}

template<typename W>
void print_stats (const W &w)
{
    auto s = w.get_samples ();
    vector<float> dx;
    vector<float> dy;
    vector<float> dz;
    vector<float> d;
    for (size_t i = 0; i + 1 < s.size (); ++i)
    {
        if (s[i].size () == 1 && s[i + 1].size () == 1)
        {
            dx.push_back (s[i][0].x - s[i + 1][0].x);
            dy.push_back (s[i][0].y - s[i + 1][0].y);
            dz.push_back (s[i][0].z - s[i + 1][0].z);
            d.push_back (distance (s[i][0], s[i + 1][0]));
        }
    }
    clog
        << "average, variance "
        << ' ' << average (dx)
        << ' ' << average (dy)
        << ' ' << average (dz)
        << ' ' << variance (dx)
        << ' ' << variance (dy)
        << ' ' << variance (dz)
        << endl;
    auto t = w.get_timestamps ();
    auto tt = (t.front () - t.back ()) / 1000000.0;
    auto dd = accumulate (d.begin (), d.end (), 0.0);
    clog
        << "distance, time, speed "
        << ' ' << dd
        << ' ' << tt
        << ' ' << dd * 1.0 / tt << "mm/s"
        << endl;
}

class soma_measure : public Leap::Listener
{
    private:
    bool done;
    sliding_time_window<points> w;
    public:
    soma_measure ()
        : done (false)
        , w (3000000)
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
        const points &p = get_points (f.pointables ());
        if (p.size () == 6)
        {
            done = true;
        }
        else
        {
            w.add_sample (f.timestamp (), p);
            if (w.fullness (f.timestamp ()) > 0.9)
            {
                print_stats (w);
                w.clear ();
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
