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

struct sample
{
};

struct observer
{
    void add (const sample &) { ++count; }
    void remove (const sample &) { --count; }
    void clear () { count = 0; }
    size_t count;
    observer ()
        : count (0)
    {
    }
};

void test_sliding_window (const bool verbose)
{
    const uint64_t D = 100;
    sample s;
    observer obs;
    sliding_window<sample> sw (D);
    vector<sample> r;
    // add with no sliding
    for (uint64_t ts = 0; ts < D; ++ts)
    {
        VERIFY (obs.count == ts);
        VERIFY (sw.size () == 100 * ts / D);
        VERIFY (sw.fullness (ts) < 1.0);
        sw.add (ts, s, obs);
    }
    if (verbose)
        clog << obs.count << ' ' << sw.size () << endl;
    // make it slide
    for (uint64_t ts = D; ts < 2 * D; ++ts)
    {
        VERIFY (obs.count == D);
        VERIFY (sw.size () == 100);
        sw.add (ts, s, obs);
        VERIFY (sw.fullness (ts) > 0.98);
    }
    if (verbose)
        clog << obs.count << ' ' << sw.size () << endl;
    // make half of them slide off
    uint64_t ts = 2 * D + D / 2;
    sw.add (ts, s, obs);
    if (verbose)
        clog << obs.count << ' ' << sw.size () << endl;
    VERIFY (obs.count == D / 2);
    VERIFY (sw.size () == 50);
    VERIFY (sw.fullness (ts) > 0.98);
    // clear it
    sw.clear ();
    obs.clear ();
    if (verbose)
        clog << obs.count << ' ' << sw.size () << endl;
    VERIFY (obs.count == 0);
    VERIFY (sw.size () == 0);
    VERIFY (sw.fullness (ts) == 0.0);
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
