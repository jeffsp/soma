/// @file dump_samples.cc
/// @brief get samples and dump to cout
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-06

#include <stdexcept>
#include "soma.h"

using namespace std;
using namespace soma;
const string usage = "usage: dump_samples <#>";

class soma_dumper : public Leap::Listener
{
    private:
    const int n;
    bool done;
    frame_counter frc;
    public:
    soma_dumper (unsigned n)
        : n (n)
        , done (false)
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
        frc.update (f.timestamp ());
        Leap::PointableList p = f.pointables ();
        if (p.count () == 6)
        {
            done = true;
        }
        else if (p.count () == n)
        {
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

        soma_dumper sd (n);
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
