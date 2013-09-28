/// @file test_finger_id_tracker.cc
/// @brief test finger_id_tracker class
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-05

#include "../finger_id_tracker.h"
#include "verify.h"
#include <deque>
#include <iostream>

using namespace std;
using namespace soma;
const string usage = "usage: test_finger_id_tracker [verbose]";

void test_finger_id_tracker1 (const bool verbose)
{
    finger_id_tracker fit (10);
    VERIFY (!fit.has_changed (1));
    fit.add (0, { 1 });
    VERIFY (fit.has_changed (1));
    fit.add (1, { 1 });
    VERIFY (!fit.has_changed (1));
    for (int i = 2; i < 5; ++i)
    {
        fit.add (i, { 1 });
        VERIFY (!fit.has_changed (1));
        VERIFY (fit.get_ids (1)[0] == 1);
    }
    // fit now has 5 1's
    for (int i = 5; i < 10; ++i)
    {
        fit.add (i, { 2 });
        VERIFY (!fit.has_changed (1));
        VERIFY (fit.get_ids (1)[0] == 1);
    }
    // fit now has 5 1's and 5 2's
    fit.add (10, { 2 });
    VERIFY (fit.has_changed (1));
    VERIFY (fit.get_ids (1)[0] == 2);
}

void test_finger_id_tracker2 (const bool verbose)
{
    std::deque<finger_ids> samples;
    const uint64_t D = 100;
    const size_t S = 10000;
    finger_id_tracker fit (D);
    if (verbose)
        clog << "generating " << S << " samples" << endl;
    for (size_t i = 0; i < S; ++i)
    {
        const size_t MAXF = 6;
        // random number of fingers
        size_t n = rand () % MAXF;
        finger_ids f (n);
        // random ids
        for (auto &j : f)
            j = rand () % MAXF;
        // save state
        vector<finger_ids> before (MAXF);
        for (size_t j = 0; j < MAXF; ++j)
            before[j] = fit.get_ids (j);
        // add it
        fit.add (i, f);
        // get new state
        vector<finger_ids> after (MAXF);
        for (size_t j = 0; j < MAXF; ++j)
            after[j] = fit.get_ids (j);
        // keep our own sliding window
        samples.push_front (f);
        if (samples.size () > D)
            samples.pop_back ();
        // for each number of fingers
        for (size_t j = 0; j < MAXF; ++j)
        {
            // get ids
            finger_ids ids0 = fit.get_ids (j);
            if (verbose && fit.has_changed (j))
            {
                for (auto k : ids0)
                    clog << " " << k;
                clog << endl;
                VERIFY (ids0 == after[j]);
                VERIFY (before[j] != after[j]);
            }
            /*
            // compare them to ours
            finger_ids ids1 (j);
            for (size_t k = 0; k < j; ++k)
            {
                std::map<size_t,size_t> dist;
                std::vector<int32_t> tmp;
                for (auto s : samples)
                {
                    if (s.size () == j)
                        ++dist[s[k]];
                    tmp.push_back (s[k]);
                }
                int32_t n0 = ids0[k];
                int32_t n1 = mode (tmp);
                VERIFY (n0 == n1 || dist[n0] == dist[n1]);
            }
            */
        }
    }
    if (verbose)
        clog << endl;
}

int main (int argc, char **)
{
    try
    {
        const bool verbose = (argc > 1);
        test_finger_id_tracker1 (verbose);
        test_finger_id_tracker2 (verbose);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
