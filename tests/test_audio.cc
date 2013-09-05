/// @file test_audio.cc
/// @brief test audio interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-03

#include "../audio.h"
#include "verify.h"
#include <chrono>
#include <iostream>

using namespace std;
using namespace chrono;
using namespace soma;
const string usage = "usage: test_audio [verbose]";

void test_audio (const bool verbose)
{
    audio a;
    if (verbose)
        clog << "playing C4" << endl;
    auto start = high_resolution_clock::now ();
    a.play (262, 300);
    auto end = high_resolution_clock::now ();
    duration<double> elapsed = end-start;
    if (verbose)
        clog << "elapsed time: " << elapsed.count () * 1000 << "ms" << endl;
    // the play() call should not block, so it should take less than 10ms
    VERIFY (elapsed.count () * 1000 < 10);
}

int main (int argc, char **)
{
    try
    {
        const bool verbose = (argc > 1);
        test_audio (verbose);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
