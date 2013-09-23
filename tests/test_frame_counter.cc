/// @file test_frame_counter.cc
/// @brief test frame_counter class
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-05

#include "../frame_counter.h"
#include "verify.h"
#include <iostream>

using namespace std;
using namespace soma;
const string usage = "usage: test_frame_counter [verbose]";

void test_frame_counter (const bool verbose)
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
        test_frame_counter (verbose);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
