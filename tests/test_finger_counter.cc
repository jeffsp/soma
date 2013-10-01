/// @file test_finger_counter.cc
/// @brief test finger_counter class
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-05

#include "../finger_counter.h"
#include "verify.h"
#include <deque>
#include <iostream>

using namespace std;
using namespace soma;
const string usage = "usage: test_finger_counter [verbose]";

void test_finger_counter1 (const bool verbose)
{
    finger_counter fc (10, 0, 0);
    VERIFY (!fc.has_changed ());
    fc.add (0, 1);
    VERIFY (fc.has_changed ());
    fc.add (1, 1);
    VERIFY (!fc.has_changed ());
    for (int i = 2; i < 5; ++i)
    {
        fc.add (i, 1);
        VERIFY (!fc.has_changed ());
        VERIFY (fc.get_count () == 1);
    }
    // fc now has 5 1's
    for (int i = 5; i < 10; ++i)
    {
        fc.add (i, 2);
        VERIFY (!fc.has_changed ());
        VERIFY (fc.get_count () == 1);
    }
    // fc now has 5 1's and 5 2's
    fc.add (10, 2);
    VERIFY (fc.has_changed ());
    VERIFY (fc.get_count () == 2);
}

void test_finger_counter2 (const bool verbose)
{
    std::deque<size_t> samples;
    const uint64_t D = 100;
    const size_t S = 10000;
    finger_counter fc (D, 0, 0);
    if (verbose)
        clog << "generating " << S << " samples" << endl;
    for (size_t i = 0; i < S; ++i)
    {
        // random number of fingers
        size_t n = rand () % 11;
        fc.add (i, n);
        // keep our own sliding window
        samples.push_front (n);
        if (samples.size () > D)
            samples.pop_back ();
        if (verbose && fc.has_changed ())
            clog << " " << fc.get_count ();
        // get count
        size_t n0 = fc.get_count ();
        // compare it to ours
        size_t n1 = mode (samples);
        // mode() may not return n0 if n0's count is the same as n1's
        std::map<size_t,size_t> dist;
        for (auto i : samples)
            ++dist[i];
        VERIFY (n0 == n1 || dist[n0] == dist[n1]);
    }
    if (verbose)
        clog << endl;
}

void test_finger_counter3 (const bool verbose)
{
    finger_counter fc (10);
    VERIFY (!fc.has_changed ());
    fc.add (0, 1);
    VERIFY (!fc.has_changed ());
    fc.add (1, 1);
    VERIFY (!fc.has_changed ());
    for (int i = 2; i < 5; ++i)
    {
        fc.add (i, 1);
        VERIFY (!fc.has_changed ());
        VERIFY (fc.get_count () == -1);
    }
    for (int i = 5; i < 10; ++i)
        fc.add (i, 1);
    VERIFY (fc.get_count () == 1);
    for (int i = 11; i < 20; ++i)
        fc.add (i, 2);
    VERIFY (fc.get_count () == 2);
}
int main (int argc, char **)
{
    try
    {
        const bool verbose = (argc > 1);
        test_finger_counter1 (verbose);
        test_finger_counter2 (verbose);
        test_finger_counter3 (verbose);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
