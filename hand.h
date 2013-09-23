/// @file hand.h
/// @brief hand definitions
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-23

#ifndef HAND_H
#define HAND_H

#include <algorithm>
#include <cstdint>
#include <limits>
#include "stats.h"
#include "Leap.h"

namespace soma
{

typedef Leap::Vector vec3;

struct finger
{
    int32_t id;
    vec3 position;
    vec3 velocity;
    vec3 direction;
    finger ()
        : id (std::numeric_limits<uint32_t>::max ())
    {
    }
};

bool sort_by_id (const finger &a, const finger &b)
{
    return a.id < b.id;
}

bool sort_left_to_right (const finger &a, const finger &b)
{
    return a.position.x < b.position.x;
}

bool sort_top_to_bottom (const finger &a, const finger &b)
{
    return a.position.y > b.position.y;
}

class hand_sample : public std::vector<finger>
{
    public:
    hand_sample (const Leap::PointableList &pl)
    {
        resize (pl.count ());
        // get the relevant info from the list
        for (int i = 0; i < pl.count () && i < 5; ++i)
        {
            data ()[i].id = pl[i].id ();
            data ()[i].position = pl[i].tipPosition ();
            data ()[i].velocity = pl[i].tipVelocity ();
            data ()[i].direction = pl[i].direction ();
        }
        // sort by x position
        std::sort (begin (), end (), sort_left_to_right);
    }
};

typedef std::vector<hand_sample> hand_samples;

hand_samples filter_by_num_fingers (const hand_samples &s)
{
    if (s.empty ())
        return s;
    // get the mode of the number of fingers
    std::vector<size_t> x (s.size ());
    for (size_t i = 0; i < x.size (); ++i)
        x[i] = s[i].size ();
    size_t nf = mode (x);
    //std::clog << "mode of number of fingers " << nf << std::endl;
    // build new vector containing only ones with correct number
    hand_samples r;
    for (auto i : s)
        if (i.size () == nf)
            r.push_back (i);
    return r;
}

hand_samples filter_by_finger_ids (const hand_samples &s)
{
    if (s.empty ())
        return s;
    // how many fingers are there?
    size_t n = s[0].size ();
    std::vector<std::vector<int32_t>> ids (n);
    for (auto i : s)
    {
        // each hand sample should have the same number of fingers
        assert (i.size () == n);
        for (size_t j = 0; j < n; ++j)
            ids[j].push_back (i[j].id);
    }
    // get the mode of each id
    std::vector<int32_t> m (n);
    for (size_t j = 0; j < n; ++j)
    {
        m[j] = mode (ids[j]);
        //std::clog << "mode of id[" << j << "] is " << m[j] << std::endl;
    }
    // build new vector containing only ones with correct ids
    hand_samples r;
    for (auto i : s)
    {
        bool good = true;
        for (size_t j = 0; j < n; ++j)
        {
            if (i[j].id != m[j])
            {
                good = false;
                break;
            }
        }
        if (good)
            r.push_back (i);
    }
    return r;
}

hand_samples filter (const hand_samples &s)
{
    // filter by mode of number of fingers
    hand_samples r = filter_by_num_fingers (s);
    // filter by finger ids
    r = filter_by_finger_ids (r);
    return r;
}

}

#endif
