/// @file test_audio.cc
/// @brief test audio interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-03

#include <iostream>
#include <sstream>
#include "verify.h"
#include "../audio.h"

using namespace std;
using namespace soma;
const string usage = "usage: test_audio";

void test_audio ()
{
    audio a;
    a.play (262, 400);
}

int main ()
{
    try
    {
        test_audio ();

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
