/// @file test_utility.cc
/// @brief test utility functions
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-05

#include "../utility.h"
#include "verify.h"
#include <iostream>

using namespace std;
using namespace soma;
const string usage = "usage: test_utility [verbose]";

void test_utility (const bool verbose)
{
    frame_counter fc;
    for (uint64_t i = 0; i <= 100; ++i)
        fc.update (i * 10000); // convert to microsecs
    float fps = fc.fps ();
    if (verbose)
        clog << "fps " << fps << endl;
    VERIFY (fps == 100);
}

int main (int argc, char **)
{
    try
    {
        const bool verbose = (argc > 1);
        test_utility (verbose);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
