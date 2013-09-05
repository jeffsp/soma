/// @file test_options.cc
/// @brief test options interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-03

const int MAJOR_REVISION = 12;
const int MINOR_REVISION = 34;

#include "../options.h"
#include "verify.h"
#include <chrono>
#include <iostream>

using namespace std;
using namespace chrono;
using namespace soma;
const string usage = "usage: test_options [verbose]";

void test_options (const bool verbose)
{
    string config_fn = get_config_dir () + "/tmprc";
    {
        options opts;
        opts.set_sound (true);
        write (opts, config_fn);
    }
    {
        options opts;
        read (opts, config_fn);
        if (verbose)
            clog << "major_revision " << opts.get_major_revision () << endl;
        if (verbose)
            clog << "minor_revision " << opts.get_minor_revision () << endl;
        if (verbose)
            clog << "sound " << opts.get_sound () << endl;
        VERIFY (opts.get_major_revision () == MAJOR_REVISION);
        VERIFY (opts.get_minor_revision () == MINOR_REVISION);
        VERIFY (opts.get_sound () == true);
    }
    {
        options opts;
        opts.set_sound (false);
        write (opts, config_fn);
    }
    {
        options opts;
        read (opts, config_fn);
        if (verbose)
            clog << "sound " << opts.get_sound () << endl;
        VERIFY (opts.get_major_revision () == MAJOR_REVISION);
        VERIFY (opts.get_minor_revision () == MINOR_REVISION);
        VERIFY (opts.get_sound () == false);
    }
}

int main (int argc, char **)
{
    try
    {
        const bool verbose = (argc > 1);
        test_options (verbose);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
