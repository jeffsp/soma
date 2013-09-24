/// @file test_sliding_window.cc
/// @brief test sliding_window class
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-23

#include "../sliding_window.h"
#include "verify.h"
#include <iostream>

using namespace std;
using namespace soma;
const string usage = "usage: test_sliding_window [verbose]";

size_t count = 0;

struct sample
{
};

struct policy
{
    static void add (const sample &) { ++count; }
    static void remove (const sample &) { --count; }
};

void test_sliding_window (const bool verbose)
{
    const uint64_t D = 100;
    sliding_window<sample,policy> sw (D);
    sample s;
    // add with no sliding
    for (uint64_t ts = 0; ts < D; ++ts)
    {
        VERIFY (count == ts);
        VERIFY (sw.size () == 100 * ts / D);
        sw.add (ts, s);
    }
    if (verbose)
        clog << count << ' ' << sw.size () << endl;
    // make it slide
    for (uint64_t ts = D; ts < 2 * D; ++ts)
    {
        VERIFY (count == D);
        VERIFY (sw.size () == 100);
        sw.add (ts, s);
    }
    if (verbose)
        clog << count << ' ' << sw.size () << endl;
    // make half of them slide off
    uint64_t ts = 2 * D + D / 2;
    sw.add (ts, s);
    if (verbose)
        clog << count << ' ' << sw.size () << endl;
    VERIFY (count == D / 2);
    VERIFY (sw.size () == 50);
    // clear it
    sw.clear ();
    if (verbose)
        clog << count << ' ' << sw.size () << endl;
    VERIFY (count == 0);
    VERIFY (sw.size () == 0);
}

int main (int argc, char **)
{
    try
    {
        const bool verbose = (argc > 1);
        test_sliding_window (verbose);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
