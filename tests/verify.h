/// @file verify.h
/// @brief verify
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-03

#ifndef VERIFY_H
#define VERIFY_H

#include <stdexcept>

namespace soma
{

inline void Verify (const char *e, const char *file, unsigned line)
{
    std::stringstream s;
    s << "verification failed in " << file << ", line " << line << ": " << e;
    throw std::runtime_error (s.str ());
}

#define VERIFY(e) (void)((e) || (Verify (#e, __FILE__, __LINE__), 0))

}

#endif
