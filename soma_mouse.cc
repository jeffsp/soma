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

        clog << "reading classifier from stdin" << endl;
        hand_shape_classifier hsc;
        cin >> hsc;

        soma_mouse sm (opts, hsc);
        Leap::Controller c (sm);

        // receive frames even when you don't have focus
        c.setPolicyFlags (Leap::Controller::POLICY_BACKGROUND_FRAMES);

        clog << "6 fingers = quit" << endl;

        while (!sm.is_done ())
            usleep (5000);

        usleep (1000000);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
