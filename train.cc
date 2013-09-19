/// @file train.cc
/// @brief train
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-14

#include "soma.h"

using namespace std;
using namespace soma;
const string usage = "usage: train";

typedef vector<hand_sample> hand_samples;

class hand_sample_grabber : public Leap::Listener
{
    private:
    bool on;
    uint64_t max_time;
    timestamps ts;
    hand_samples hs;
    public:
    hand_sample_grabber ()
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
        hs.push_back (hand_sample (f.pointables ()));
        ts.push_back (f.timestamp ());
        if (ts.back () - ts.front () >= max_time)
            on = false;
    }
    const hand_samples &get_hand_samples ()
    {
        return hs;
    }
    const timestamps &get_timestamps ()
    {
        return ts;
    }
    void grab (uint64_t usec)
    {
        assert (!on);
        // start with no frames
        hs.clear ();
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

void train (hand_sample_grabber &g, hand_shape_classifier &hsc, const hand_shape hs)
{
    const uint64_t SAMPLE_DURATION = 1000000;
    // display hand position
    string hss = to_string (hs);
    std::transform (hss.begin(), hss.end(), hss.begin(), ::toupper);
    clog << "training hand position " << hss << endl;
    // continue after user input
    clog << "press enter" << endl;
    string str;
    getline (cin, str);
    clog << "getting feature vectors... ";
    // grab some samples
    g.grab (SAMPLE_DURATION);
    hand_samples s = g.get_hand_samples ();
    clog << "got " << s.size () << " samples" << endl;
    // filter out bad samples
    hand_samples fs = filter (s);
    assert (fs.size () <= s.size ());
    const size_t nf = s.size () - fs.size ();
    clog << "filtered out " << nf << " samples" << endl;
    // make sure they are reliable samples
    if (100 * nf / s.size () > 75)
        throw runtime_error ("filtered out too many samples");
    // convert them to feature vectors
    hand_shape_feature_vectors fv (fs.begin (), fs.end ());
    // record them
    hsc.update (hs, fv);
}

void classify (hand_sample_grabber &g, const hand_shape_classifier &hsc)
{
    const uint64_t SAMPLE_DURATION = 100000;
    while (1)
    {
        // get some frames
        g.grab (SAMPLE_DURATION);
        hand_samples s = g.get_hand_samples ();
        timestamps ts = g.get_timestamps ();
        assert (!s.empty ());
        assert (s.size () == ts.size ());
        // filter out bad samples
        hand_samples fs = filter (s);
        const size_t nf = s.size () - fs.size ();
        clog << "filtered out " << nf << " samples" << endl;
        // convert them to feature vectors
        hand_shape_feature_vectors fv (fs.begin (), fs.end ());
        // classify them
        hand_shape hs;
        double p;
        hsc.classify (fv, ts, hs, p);
        // show the class you got and its likelihood
        string hss = to_string (hs);
        std::transform (hss.begin(), hss.end(), hss.begin(), ::toupper);
        clog << "class = " << hss << " (" << p << ")" << endl;
    }
}

int main (int argc, char **argv)
{
    try
    {
        if (argc != 1)
            throw runtime_error (usage);

        hand_sample_grabber g;
        Leap::Controller c (g);
        hand_shape_classifier hsc;

        const size_t PASSES = 1;
        for (size_t pass = 0; pass < PASSES; ++pass)
        {
            clog << "pass " << pass << endl;

            vector<hand_shape> vhs { hand_shape::pointing, hand_shape::clicking, hand_shape::scrolling, hand_shape::centering };
            for (auto hs : vhs)
                train (g, hsc, hs);
        }

        classify (g, hsc);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
