/// @file classify.cc
/// @brief classify
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-20

#include "soma.h"

using namespace std;
using namespace soma;
const string usage = "usage: classify";

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
        // end if you show 6 or more fingers
        if (!fs.empty () && fs[0].size () > 5)
            break;
        //const size_t nf = s.size () - fs.size ();
        //clog << "filtered out " << nf << " samples" << endl;
        // convert them to feature vectors
        hand_shape_feature_vectors fv (fs.begin (), fs.end ());
        // classify them
        map<hand_shape,double> l;
        hsc.classify (fv, ts, l);
        double best_value = numeric_limits<int>::min ();
        hand_shape best_hs = hand_shape::unknown;
        for (auto i : l)
        {
            if (i.second > best_value)
            {
                best_hs = i.first;
                best_value = i.second;
            }
        }
        static hand_shape last_hs = hand_shape::unknown;
        if (last_hs != best_hs)
        {
            // show the class you got and its likelihood
            string hss = to_string (best_hs);
            transform (hss.begin(), hss.end(), hss.begin(), ::toupper);
            clog << "class = " << hss << " (" << best_value << ")" << endl;
            for (auto i : l)
                clog << to_string (i.first) << ' ' << i.second << endl;
            last_hs = best_hs;
        }
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

        cin >> hsc;

        classify (g, hsc);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
