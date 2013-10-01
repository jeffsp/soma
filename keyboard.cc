/// @file keyboard.cc
/// @brief keybaord test module
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-01

#include "keyboard.h"
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;
using namespace soma;

int main ()
{
    try
    {
        keyboard k;
        while (1)
        {
            vector<int> s = k.key_states ();
            for (auto key : s)
                clog << ' ' << key;
            clog << endl;
        }

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
