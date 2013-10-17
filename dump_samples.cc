/// @file dump_samples.cc
/// @brief get samples and dump to cout
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-06

#include "frame_counter.h"
#include "Leap.h"
#include <stdexcept>

using namespace std;
using namespace soma;
const string usage = "usage: dump_samples <#_fingers>";

/// @brief dump samples to cout
class dumper : public Leap::Listener
{
    private:
    const int n;
    bool done;
    frame_counter frc;
    uint64_t start_ts;
    public:
    /// @brief constructor
    ///
    /// @param n number of fingers
    dumper (unsigned n)
        : n (n)
        , done (false)
        , start_ts (0)
    {
    }
    /// @brief check if we have exited
    ///
    /// @return true is exited
    bool is_done () const
    {
        return done;
    }
    /// @brief get a frame and dump its contents
    ///
    /// @param c leap controller
    virtual void onFrame(const Leap::Controller& c)
    {
        if (done)
            return;
        const Leap::Frame &f = c.frame ();
        uint64_t ts = f.timestamp ();
        if (start_ts == 0)
            start_ts = ts;
        frc.update (ts);
        Leap::PointableList p = f.pointables ();
        if (p.count () == 6)
        {
            // we are done
            done = true;
        }
        else if (p.count () == n)
        {
            // dump the samples
            cout << ts - start_ts;
            for (int i = 0; i < p.count (); ++i)
            {
                cout << ' ' << p[i].tipPosition ().x;
                cout << ' ' << p[i].tipPosition ().y;
                cout << ' ' << p[i].tipPosition ().z;
            }
            cout << endl;
            if (!(frc.get_frames () % 100))
                std::clog << frc.fps () << "fps" << std::endl;
        }
    }
};

int main (int argc, char **argv)
{
    try
    {
        if (argc != 2)
            throw runtime_error (usage);

        int n = atoi (argv[1]);

        clog << "n " << n << endl;

        dumper sd (n);
        Leap::Controller c (sd);

        clog << "6 fingers = quit" << endl;
        clog << "dumping..." << endl;

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
