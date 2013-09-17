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
        if (ts.back () - ts.front () >= max_time)
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

void train (frame_grabber &fg, hand_position_classifier &hpc, const hand_position hp)
{
    const uint64_t SAMPLE_DURATION = 10000000;
    // display hand position
    string hps = to_string (hp);
    std::transform (hps.begin(), hps.end(), hps.begin(), ::toupper);
    clog << "training hand position " << hps << endl;
    // continue after user input
    clog << "press enter" << endl;
    string str;
    getline (cin, str);
    clog << "getting feature vectors... ";
    // grab some vectors
    fg.grab (SAMPLE_DURATION);
    feature_vectors fvs = fg.get_feature_vectors ();
    clog << "got " << fvs.size () << " samples" << endl;
    // record them
    hpc.update (hp, fvs);
}

void classify (frame_grabber &fg, const hand_position_classifier &hpc)
{
    const uint64_t SAMPLE_DURATION = 100000;
    while (1)
    {
        // get some frames
        fg.grab (SAMPLE_DURATION);
        feature_vectors fvs = fg.get_feature_vectors ();
        timestamps ts = fg.get_timestamps ();
        assert (!fvs.empty ());
        assert (fvs.size () == ts.size ());
        // classify them
        hand_position hp;
        double p;
        hpc.classify (fvs, ts, hp, p);
        // show the class you got and its likelihood
        string hps = to_string (hp);
        std::transform (hps.begin(), hps.end(), hps.begin(), ::toupper);
        clog << "class = " << hps << " (" << p << ")" << endl;
    }
}

int main (int argc, char **argv)
{
    try
    {
        if (argc != 1)
            throw runtime_error (usage);

        frame_grabber fg;
        Leap::Controller c (fg);
        hand_position_classifier hpc;

        const size_t PASSES = 1;
        for (size_t pass = 0; pass < PASSES; ++pass)
        {
            clog << "pass " << pass << endl;

            vector<hand_position> vhp { hand_position::pointing, hand_position::clicking, hand_position::scrolling, hand_position::centering };
            for (auto hp : vhp)
                train (fg, hpc, hp);
        }

        classify (fg, hpc);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
