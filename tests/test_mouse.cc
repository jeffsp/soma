/// @file test_mouse.cc
/// @brief test mouse interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-05

#include "../mouse.h"
#include "verify.h"
#include <iostream>

using namespace std;
using namespace soma;
const string usage = "usage: test_mouse [verbose]";

void test_mouse (const bool verbose)
{
    mouse m;
    if (verbose)
        clog << "moving mouse left 100 pixels" << endl;
    m.move (-100, 0);
    usleep (100000);
    if (verbose)
        clog << "moving mouse up 100 pixels" << endl;
    m.move (0, -100);
    usleep (100000);
    if (verbose)
        clog << "moving mouse right 100 pixels" << endl;
    m.move (100, 0);
    usleep (100000);
    if (verbose)
        clog << "moving mouse down 100 pixels" << endl;
    m.move (0, 100);
    usleep (100000);
}

int main (int argc, char **)
{
    try
    {
        const bool verbose = (argc > 1);
        test_mouse (verbose);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
