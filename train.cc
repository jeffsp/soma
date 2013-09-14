/// @file train.cc
/// @brief train
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-14

#include "soma.h"

using namespace std;
using namespace soma;
const string usage = "usage: train";

class frame_grabber : public Leap::Listener
{
    private:
    bool on;
    uint64_t max_time;
    timestamps ts;
    feature_vectors fvs;
    public:
    frame_grabber ()
        : on (false)
        , max_time (0)
    {
    }
    virtual void onFrame (const Leap::Controller& c)
    {
        // don't do anything if it's off
        if (!on)
            return;
        // get the frame
        Leap::Frame f = c.frame ();
        // save the frame info
        fvs.push_back (feature_vector (f.pointables ()));
        ts.push_back (f.timestamp ());
        if (ts.back () - ts.front () > max_time)
            on = false;
    }
    const feature_vectors &get_feature_vectors ()
    {
        return fvs;
    }
    const timestamps &get_timestamps ()
    {
        return ts;
    }
    void grab (uint64_t usec)
    {
        assert (!on);
        // start with no frames
        fvs.clear ();
        ts.clear ();
        // set the time
        max_time = usec;
        // start getting frames
        on = true;
        // wait until it is done
        while (on)
            usleep (1000);
    }
};

int main (int argc, char **argv)
{
    try
    {
        if (argc != 1)
            throw runtime_error (usage);

        frame_grabber fg;
        Leap::Controller c (fg);

        const size_t PASSES = 10;

        for (size_t pass = 0; pass < PASSES; ++pass)
        {
            clog << "pass " << pass << endl;

            hand_position hp = hand_position::pointing;
            string hps = to_string (hp);
            std::transform (hps.begin(), hps.end(), hps.begin(), ::toupper);
            clog << "training hand position " << hps << endl;
            clog << "press enter" << endl;
            string str;
            getline (cin, str);
            clog << "getting feature vectors... ";
            fg.grab (1000000);
            feature_vectors fvs = fg.get_feature_vectors ();
            timestamps ts = fg.get_timestamps ();
            assert (fvs.size () == ts.size ());
            clog << "got " << fvs.size () << " samples" << endl;
            hand_position_classifier hpc;
            hpc.update (hp, fvs, ts);
        }

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
