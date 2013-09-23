/// @file soma.h
/// @brief soma software for Leap controller
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-08-26

#ifndef SOMA_H
#define SOMA_H

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <unistd.h>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "stats.h"
#include "frame_counter.h"
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

size_t hand_shape_dimensions (size_t fingers)
{
    return 1 + fingers * 3 + (fingers - 1) * 4;
}

class hand_shape_features : public std::vector<double>
{
    public:
    hand_shape_features (const hand_sample &h)
    {
        push_back (h.size ());
        for (size_t i = 0; i < h.size (); ++i)
        {
            push_back (h[i].direction.x);
            push_back (h[i].direction.y);
            push_back (h[i].direction.z);
            if (i + 1 < h.size ())
            {
                float dist = h[i].position.distanceTo (h[i + 1].position);
                push_back (dist);
                vec3 dir = h[i].position - h[i + 1].position;
                push_back (dir.x);
                push_back (dir.y);
                push_back (dir.z);
            }
        }
    }
};

typedef std::vector<uint64_t> timestamps;
typedef std::vector<hand_shape_features> hand_shape_feature_vectors;
typedef int hand_shape;

class hand_shape_feature_dist
{
    private:
    size_t total;
    std::vector<double> u1;
    std::vector<double> u2;
    public:
    void resize (size_t n)
    {
        u1.resize (n);
        u2.resize (n);
    }
    size_t size () const
    {
        assert (u1.size () == u2.size ());
        return u1.size ();
    }
    void update (size_t i, const double v)
    {
        assert (i < u1.size ());
        assert (u1.size () == u2.size ());
        ++total;
        u1[i] += v;
        u2[i] += v * v;
    }
    double mean (size_t i) const
    {
        assert (i < u1.size ());
        if (total == 0)
            return 0.0;
        return u1[i] / total;
    }
    double variance (size_t i) const
    {
        assert (i < u2.size ());
        if (total == 0)
            return 0.0;
        double u = mean (i);
        assert (u2[i] / total >= u * u);
        return u2[i] / total - u * u;
    }
    friend std::ostream& operator<< (std::ostream &s, const hand_shape_feature_dist &x)
    {
        s << x.total << std::endl;
        assert (x.u1.size () == x.u2.size ());
        s << x.u1.size () << std::endl;
        s.precision (12);
        for (size_t i = 0; i < x.u1.size (); ++i)
            s << ' ' << x.u1[i] << ' ' << x.u2[i];
        return s;
    }
    friend std::istream& operator>> (std::istream &s, hand_shape_feature_dist &x)
    {
        s >> x.total;
        size_t n;
        s >> n;
        x.resize (n);
        for (size_t i = 0; i < x.u1.size (); ++i)
            s >> x.u1[i] >> x.u2[i];
        return s;
    }
};

const std::vector<hand_shape> hand_shapes { 0, 1, 2, 3, 4 };

class hand_shape_classifier
{
    private:
    typedef std::map<hand_shape,hand_shape_feature_dist> map_hand_shape_dists;
    std::vector<map_hand_shape_dists> hss;
    public:
    hand_shape_classifier ()
        : hss (5)
    {
        // for each number of fingers
        for (size_t i = 0; i < hss.size (); ++i)
            // for each hand shape
            for (auto hs : hand_shapes)
                // resize the vector of stats to the number of dimensions
                hss[i][hs].resize (hand_shape_dimensions (i + 1));
    }
    void update (const hand_shape hs, const hand_shape_feature_vectors &hsfvs)
    {
        for (auto s : hsfvs)
        {
            // first dimension always contains the number of fingers
            const size_t fingers = s[0];
            // if there are no fingers detected, there is nothing to do
            if (fingers == 0)
                return;
            const size_t map_index = fingers - 1;
            assert (map_index < hss.size ());
            auto &v = hss[map_index][hs];
            //std::clog << "hand shape feature vector dimensions " << s.size () << std::endl;
            //std::clog << "hand shape dists vector dimensions " << v.size () << std::endl;
            assert (s.size () == v.size ());
            for (size_t i = 0; i < s.size (); ++i)
            {
                v.update (i, s[i]);
                double m = v.mean (i);
                double s = v.variance (i);
                std::clog << i << ' ' << m << ' ' << sqrt (s) << std::endl;
            }
        }
    }
    void classify (const hand_shape_feature_vectors &hsfvs, std::map<hand_shape,double> &l) const
    {
        for (auto hs : hand_shapes)
        {
            size_t total = 0;
            for (auto s : hsfvs)
            {
                // first dimension always contains the number of fingers
                const size_t fingers = s[0];
                // if there are no fingers detected, there is nothing to do
                if (fingers == 0)
                    return;
                const size_t map_index = fingers - 1;
                assert (map_index < hss.size ());
                auto v = hss[map_index].find (hs);
                assert (v != hss[map_index].end ());
                assert (s.size () == v->second.size ());
                for (size_t i = 0; i < s.size (); ++i)
                {
                    const double x = s[i]; // feature dimension value
                    const double m = v->second.mean (i);
                    const double s = v->second.variance (i);
                    if (s != 0.0)
                    {
                        // update log likelihood
                        l[hs] -= (x - m) * (x - m) / (2 * s);
                        ++total;
                    }
                }
            }
            l[hs] /= total;
        }
    }
    friend std::ostream& operator<< (std::ostream &s, const hand_shape_classifier &h)
    {
        for (auto i : h.hss)
            for (auto j : i)
                s << j.second << std::endl;
        return s;
    }
    friend std::istream& operator>> (std::istream &s, hand_shape_classifier &h)
    {
        for (size_t i = 0; i < h.hss.size (); ++i)
        {
            for (auto hs : hand_shapes)
            {
                hand_shape_feature_dist x;
                s >> x;
                h.hss[i][hs] = x;
            }
        }
        return s;
    }
};

typedef std::vector<hand_sample> hand_samples;

class hand_sample_grabber : public Leap::Listener
{
    private:
    bool on;
    uint64_t max_time;
    timestamps ts;
    hand_samples hs;
    public:
    hand_sample_grabber ()
        : on (false)
        , max_time (0)
    {
    }
    virtual void onFrame (const Leap::Controller& c)
    {
        // don't do anything if it's off
        if (!on)
            return;
        // get the frame
        Leap::Frame f = c.frame ();
        // save the frame info
        hs.push_back (hand_sample (f.pointables ()));
        ts.push_back (f.timestamp ());
        if (ts.back () - ts.front () >= max_time)
            on = false;
    }
    const hand_samples &get_hand_samples ()
    {
        return hs;
    }
    const uint64_t current_timestamp () const
    {
        if (ts.empty ())
            return 0;
        return ts.back ();
    }
    const timestamps &get_timestamps ()
    {
        return ts;
    }
    void grab (uint64_t usec)
    {
        assert (!on);
        // start with no frames
        hs.clear ();
        ts.clear ();
        // set the time
        max_time = usec;
        // start getting frames
        on = true;
        // wait until it is done
        while (on)
            usleep (1000);
    }
};

}

#endif
