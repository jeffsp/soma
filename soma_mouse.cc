/// @file soma_mouse.cc
/// @brief soma mouse interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-08-30

#include "soma_mouse.h"

using namespace std;
using namespace soma;
const string usage = "usage: soma_mouse";

int main (int argc, char **argv)
{
    try
    {
        if (argc != 1)
            throw runtime_error (usage);

        // options get saved here
        string config_fn = get_config_dir () + "/somarc";

        // configurable options
        options opts;

        // if the config file does not exist, write one
        {
            std::clog << "writing default configuration file " << config_fn << std::endl;
            ifstream ifs (config_fn.c_str ());
            if (!ifs)
                write (opts, config_fn);
        }

        // read in the config file
        {
            std::clog << "reading configuration file " << config_fn << std::endl;
            ifstream ifs (config_fn.c_str ());
            if (!ifs) // if it's not there, notify the user
                clog << "warning: could not read configuration options" << endl;
            else
                read (opts, config_fn);
        }

        soma_mouse sm (opts);
        hand_sample_grabber g;
        Leap::Controller c (g);
        hand_shape_classifier hsc;

        clog << "reading classifier from stdin" << endl;
        cin >> hsc;

        // receive frames even when you don't have focus
        c.setPolicyFlags (Leap::Controller::POLICY_BACKGROUND_FRAMES);

        clog << "press control-C to quit" << endl;

        const uint64_t SAMPLE_DURATION = 20000;
        while (1)
        {
            // get some frames
            g.grab (SAMPLE_DURATION);
            hand_samples s = g.get_hand_samples ();
            // did we get anything?
            if (s.empty ())
                continue;
            // filter out bad samples
            hand_samples fs = filter (s);
            const size_t nf = s.size () - fs.size ();
            // make sure they are reliable samples
            if (100 * nf / s.size () > 25) // more than 25%?
                continue;
            // end if you show 6 or more fingers
            if (!fs.empty () && fs[0].size () > 5)
                break;
            // get next samples if these are bad
            // convert them to feature vectors
            hand_shape_feature_vectors fv (fs.begin (), fs.end ());
            // classify them
            unordered_map<hand_shape,double> l;
            hsc.classify (fv, l);
            double best_value = numeric_limits<int>::min ();
            hand_shape best_hs = -1;
            for (auto i : l)
            {
                if (i.second > best_value)
                {
                    best_hs = i.first;
                    best_value = i.second;
                }
            }
            // update
            sm.update (g.current_timestamp (), best_hs, fs[0]);
        }

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
