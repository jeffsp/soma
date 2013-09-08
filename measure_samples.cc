/// @file measure_samples.cc
/// @brief get samples and measure stats
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-06

#include "soma.h"

using namespace std;
using namespace soma;
const string usage = "usage: measure_samples";

template<typename T>
void print_stats (const T &s)
{
    clog << s.size () << endl;
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
                print_stats (w.get_samples ());
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
