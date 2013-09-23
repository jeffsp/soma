/// @file test_stats.cc
/// @brief test stats functions
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-05

#include "../stats.h"
#include "verify.h"
#include <iostream>

using namespace std;
using namespace soma;
const string usage = "usage: test_stats [verbose]";

void test_stats (const bool verbose)
{
    vector<int> x { 0, 1, 0, 6, 3, 2, 7, 4, 5, 2, 2, 6, 1 };
    if (verbose)
        clog << "mode(x)=" << mode (x) << endl;
    VERIFY (mode (x) == 2);
    if (verbose)
        clog << "mean(x)=" << mean(x) << endl;
    VERIFY (mean (x) == 3);
    if (verbose)
        clog << "variance(x)=" << variance(x) << endl;
    VERIFY (round (variance (x) * 100) == 523);
}

void test_running_stats (const bool verbose)
{
    running_mean a;
    a.add (1); a.add (2); a.add (3); a.add (4); a.add (5);
    if (verbose)
        clog << "running_mean=" << a.mean () << endl;
    VERIFY (a.mean () == 3);
    a.add (6); a.add (7);
    if (verbose)
        clog << "running_mean=" << a.mean () << endl;
    VERIFY (a.mean () == 4);
    a.remove (1); a.remove (2);
    if (verbose)
        clog << "running_mean=" << a.mean () << endl;
    VERIFY (a.mean () == 5);
    running_mode b;
    b.add (7); b.add (1); b.add (3); b.add (4); b.add (3); b.add (3); b.add (7); b.add (9);
    if (verbose)
        clog << "running_mode=" << b.mode () << endl;
    VERIFY (b.mode () == 3);
    b.add (7); b.add (7);
    if (verbose)
        clog << "running_mode=" << b.mode () << endl;
    VERIFY (b.mode () == 7);
    // both 3 and 7 are the mode-- the mode shouldn't change
    b.remove (7);
    if (verbose)
        clog << "running_mode=" << b.mode () << endl;
    VERIFY (b.mode () == 7);
    b.remove (7);
    if (verbose)
        clog << "running_mode=" << b.mode () << endl;
    VERIFY (b.mode () == 3);
}

int main (int argc, char **)
{
    try
    {
        const bool verbose = (argc > 1);
        test_stats (verbose);
        test_running_stats (verbose);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
