/// @file dump_samples.cc
/// @brief get samples and dump to cout
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-06

#include "soma.h"
#include "utility.h"

using namespace std;
using namespace soma;
const string usage = "usage: dump_samples";

class soma_dumper : public Leap::Listener
{
    private:
    bool done;
    const size_t max_points;
    frame_counter frc;
    public:
    soma_dumper ()
        : done (false)
        , max_points (5)
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
        const points &p = get_points (f.pointables ());
        if (p.size () == 6)
        {
            done = true;
        }
        else
        {
            cout << p.size ();
            for (size_t i = 0; i < max_points; ++i)
            {

                if (i < p.size ())
                    cout << ' ' << p[i];
                else
                    cout << ' ' << point ();
            }
            cout << endl;
            if (!(frc.get_frames () % 100))
                std::clog << frc.fps () << "fps" << std::endl;
        }
    }
};

int main (int argc, char **)
{
    try
    {
        if (argc != 1)
            throw runtime_error (usage);

        soma_dumper sd;
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
