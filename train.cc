/// @file train.cc
/// @brief train
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-14

#include "soma.h"

using namespace std;
using namespace soma;
const string usage = "usage: train";

void train (hand_sample_grabber &g, hand_shape_classifier &hsc, const hand_shape hs)
{
    const uint64_t SAMPLE_DURATION = 3000000;
    // display hand position
    clog << "training hand position " << hs << endl;
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
    if (100 * nf / s.size () > 25)
        throw runtime_error ("filtered out too many samples");
    // convert them to feature vectors
    hand_shape_feature_vectors fv (fs.begin (), fs.end ());
    // record them
    hsc.update (hs, fv);
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

            for (auto hs : hand_shapes)
                train (g, hsc, hs);
        }

        cout << hsc;

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
