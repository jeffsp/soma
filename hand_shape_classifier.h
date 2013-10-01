/// @file hand_shape_classifier.h
/// @brief hand_shape_classifier class
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-23

#ifndef HAND_SHAPE_CLASSIFIER_H
#define HAND_SHAPE_CLASSIFIER_H

#include "hand_sample.h"
#include "finger_counter.h"
#include <string>

namespace soma
{

enum class hand_shape
{
    unknown = -1,
    zero = 0,
    pointing = 1,
    clicking = 2,
    scrolling = 3,
    ok = 4,
    center = 5,
};

std::string to_string (const hand_shape s)
{
    switch (s)
    {
        default: assert (0); // logic error
        case hand_shape::unknown: return std::string ("unknown");
        case hand_shape::zero: return std::string ("zero");
        case hand_shape::pointing: return std::string ("pointing");
        case hand_shape::clicking: return std::string ("clicking");
        case hand_shape::scrolling: return std::string ("scrolling");
        case hand_shape::ok: return std::string ("ok");
        case hand_shape::center: return std::string ("center");
    }
}

class hand_shape_classifier
{
    private:
    finger_counter fc;
    hand_shape current;
    bool changed;
    void update ()
    {
        if (fc.has_changed ())
        {
            switch (fc.get_count ())
            {
                default:
                    current = hand_shape::unknown;
                break;
                case 0:
                    current = hand_shape::zero;
                break;
                case 1:
                    current = hand_shape::pointing;
                break;
                case 2:
                    current = hand_shape::scrolling;
                break;
                case 3:
                    current = hand_shape::ok;
                break;
                case 4:
                    current = hand_shape::unknown;
                break;
                case 5:
                    current = hand_shape::center;
                break;
            }
        }
    }
    public:
    hand_shape_classifier (uint64_t duration)
        : fc (duration)
        , current (hand_shape::unknown)
        , changed (false)
    {
    }
    void add (uint64_t ts, const hand_sample &s)
    {
        hand_shape last = current;
        fc.add (ts, s.size ());
        update ();
        changed = (last != current);
    }
    hand_shape get_shape () const
    {
        return current;
    }
    bool has_changed () const
    {
        return changed;
    }
};

}

#endif
