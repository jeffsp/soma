/// @file sample_dumper.cc
/// @brief get samples and dump to cout
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-06

#include "frame_counter.h"
#include "Leap.h"
#include <stdexcept>

using namespace std;
using namespace soma;
const string usage = "usage: sample_dumper";

/// @brief dump samples to cout
class sample_dumper : public Leap::Listener
{
    private:
    bool done;
    frame_counter frc;
    uint64_t start_ts;
    public:
    /// @brief constructor
    sample_dumper ()
        : done (false)
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
        else if (p.count () == 2)
        {
            // dump the samples
            cout << ts - start_ts;
            double d = p[0].tipPosition ().distanceTo (p[1].tipPosition ());
            cout << ' ' << d << endl;
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

        sample_dumper sd;
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
