/// @file keyboard.h
/// @brief detect key presses by polling
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-01

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <linux/input.h>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef KEYBOARD_H
#define KEYBOARD_H

namespace soma
{

class keyboard
{
    private:
    FILE *fp;
    public:
    keyboard (const std::string &fn = std::string ("/dev/input/event8"))
    {
        std::clog << "opening " << fn.c_str () << std::endl;
        fp = fopen (fn.c_str (), "r");
        if (!fp)
            throw std::runtime_error ("can't open file for reading");
    }
    ~keyboard ()
    {
        fclose (fp);
    }
    std::vector<int> key_states () const
    {
        const std::vector<int> keys {
            KEY_LEFTSHIFT,
            KEY_RIGHTSHIFT,
            KEY_LEFTALT,
            KEY_RIGHTALT,
            KEY_LEFTCTRL,
            KEY_RIGHTCTRL
            };
        std::vector<int> states (keys.size ());
        // Create a byte array the size of the number of keys
        std::vector<char> key_map (KEY_MAX/8 + 1);
        // Fill the keymap with the current keyboard state
        ioctl (fileno(fp), EVIOCGKEY (key_map.size ()), &key_map[0]);
        for (size_t i = 0; i < keys.size (); ++i)
        {
            // The key we want (and the seven others arround it)
            int bits = key_map[keys[i]/8];
            // Key's mask
            int mask = 1 << (keys[i] % 8);
            states[i] = !!(bits & mask);
        }
        return states;
    }
};

}

#endif
