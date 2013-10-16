/// @file hand_shape_classifier.h
/// @brief hand_shape_classifier class
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-23

#ifndef HAND_SHAPE_CLASSIFIER_H
#define HAND_SHAPE_CLASSIFIER_H

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
    static const uint64_t D = 100000;
    sliding_window<hand_sample> swhs;
    hand_shape current;
    bool changed;
    void update_finger_count2 (const hand_sample &s)
    {
        // if the sample does not have two fingers, we are in transition, so don't change anything
        if (s.size () != 2)
            return;
        // get slope of two fingers in xy plane
        vec3 p0 = s[0].position;
        vec3 p1 = s[1].position;
        // make sure first is lower than second
        if (p0.y > p1.y)
            std::swap (p0, p1);
        // get the angle
        float dx = abs (p0.x- p1.x);
        float dy = abs (p0.y- p1.y);
        float slope = atan2 (dy, dx) * 180 / M_PI;
        if (slope < 45)
            current = hand_shape::scrolling;
        else
            current = hand_shape::pointing;
    }
    void update (const hand_sample &s)
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
                update_finger_count2 (s);
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
    public:
    hand_shape_classifier (uint64_t duration)
        : fc (duration)
        , swhs (D)
        , current (hand_shape::unknown)
        , changed (false)
    {
    }
    void add (uint64_t ts, const hand_sample &s)
    {
        hand_shape last = current;
        fc.add (ts, s.size ());
        swhs.add (ts, s);
        update (s);
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
