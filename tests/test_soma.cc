/// @file test_soma.cc
/// @brief test soma leap interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-08-29

#include <sstream>
#include "soma.h"

using namespace std;
using namespace soma;
const string usage = "usage: test_soma [verbose]";

inline void Verify (const char *e, const char *file, unsigned line)
{
    std::stringstream s;
    s << "verification failed in " << file << ", line " << line << ": " << e;
    throw std::runtime_error (s.str ());
}

#define VERIFY(e) (void)((e) || (Verify (#e, __FILE__, __LINE__), 0))

void test_stats (const bool verbose)
{
    vector<int> x { 0, 1, 0, 6, 3, 2, 7, 4, 5, 2, 2, 6, 1 };
    if (verbose)
        clog << "mode(x)=" << mode (x) << endl;
    VERIFY (mode (x) == 2);
    if (verbose)
        clog << "average(x)=" << average(x) << endl;
    VERIFY (average (x) == 3);
    if (verbose)
        clog << "variance(x)=" << variance(x) << endl;
    VERIFY (round (variance (x) * 100) == 523);
}

int main (int argc, char **)
{
    try
    {
        const bool verbose = (argc > 1);

        test_stats (verbose);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
