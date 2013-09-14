/// @file train.cc
/// @brief train
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-14

#include "soma.h"

using namespace std;
using namespace soma;
const string usage = "usage: train";

int main (int argc, char **argv)
{
    try
    {
        if (argc != 1)
            throw runtime_error (usage);

        hand_position_classifier c;

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
