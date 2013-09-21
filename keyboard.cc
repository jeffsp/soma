#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <linux/input.h>
#include <stdexcept>
#include <vector>

using namespace std;

int main ()
{
    try
    {
        const char *fn = "/dev/input/event4";
        clog << "opening " << fn << endl;

        FILE *kbd = fopen (fn, "r");

        if (!kbd)
            throw runtime_error ("can't open file for reading");

        vector<int> keys {
            KEY_LEFTSHIFT,
            KEY_RIGHTSHIFT,
            KEY_LEFTALT,
            KEY_RIGHTALT,
            KEY_LEFTCTRL,
            KEY_RIGHTCTRL
            };

        while (1)
        {
            clog << "---" << endl;
            vector<char> key_map (KEY_MAX/8 + 1);    //  Create a byte array the size of the number of keys
            ioctl (fileno(kbd), EVIOCGKEY (key_map.size ()), &key_map[0]);    //  Fill the keymap with the current keyboard state

            for (auto key : keys)
            {
                int keyb = key_map[key/8];  //  The key we want (and the seven others arround it)
                int mask = 1 << (key % 8);  //  Put a one in the same column as out key state will be in;
                clog << ' ' << !(keyb & mask);
            }
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
