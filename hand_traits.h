/// @file hand_traits.h
/// @brief hand traits
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-05

#ifndef HAND_TRAITS_H
#define HAND_TRAITS_H

namespace soma
{

enum class size : int { small, big };

template<size H>
struct hand_traits
{
    static constexpr double pinch_min = 1.0;
    static constexpr double pinch_max = 1.0;
};

template<>
struct hand_traits<size::small>
{
    static constexpr double pinch_min = 1.0;
    static constexpr double pinch_max = 1.0;
};

template<>
struct hand_traits<size::big>
{
    static constexpr double pinch_min = 1.0;
    static constexpr double pinch_max = 1.0;
};

}

#endif
