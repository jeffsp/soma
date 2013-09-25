/// @file hand_shape_classifier.h
/// @brief hand_shape_classifier class
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-23

#ifndef HAND_SHAPE_CLASSIFIER_H
#define HAND_SHAPE_CLASSIFIER_H

#include "hand.h"
#include <limits>

namespace soma
{

size_t hand_shape_dimensions (size_t fingers)
{
    if (fingers == 0)
        return 1;
    else
        return 1 + fingers * 3 + (fingers - 1) * 4;
}

class hand_shape_features : public std::vector<double>
{
    public:
    hand_shape_features (const hand_sample &h)
    {
        size_t fingers = h.size ();
        push_back (fingers);
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
        assert (size () == hand_shape_dimensions (fingers));
    }
};

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

typedef int hand_shape;
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
    void update (const hand_shape hs, const hand_shape_features &hsf)
    {
        // first dimension always contains the number of fingers
        const size_t fingers = hsf[0];
        // if there are no fingers detected, there is nothing to do
        if (fingers == 0)
            return;
        const size_t map_index = fingers - 1;
        assert (map_index < hss.size ());
        auto &v = hss[map_index][hs];
        //std::clog << "hand shape feature vector dimensions " << s.size () << std::endl;
        //std::clog << "hand shape dists vector dimensions " << v.size () << std::endl;
        assert (hsf.size () == v.size ());
        for (size_t i = 0; i < hsf.size (); ++i)
        {
            v.update (i, hsf[i]);
            double m = v.mean (i);
            double s = v.variance (i);
            std::clog << i << ' ' << m << ' ' << sqrt (s) << std::endl;
        }
    }
    hand_shape classify (const std::vector<hand_shape_features> &hsf, std::map<hand_shape,double> &l) const
    {
        for (auto s : hsf)
        {
            // first dimension always contains the number of fingers
            const size_t fingers = s[0];
            // if there are no fingers detected, there is nothing to do
            if (fingers == 0)
                return -1;
            for (auto hs : hand_shapes)
            {
                const size_t map_index = fingers - 1;
                assert (map_index < hss.size ());
                auto d = hss[map_index].find (hs);
                assert (d != hss[map_index].end ());
                assert (s.size () == d->second.size ());
                for (size_t i = 1; i < s.size (); ++i)
                {
                    const double x = s[i]; // feature dimension value
                    const double m = d->second.mean (i);
                    const double s = d->second.variance (i);
                    // update log likelihood
                    if (s != 0.0)
                        l[hs] -= (x - m) * (x - m) / (2 * s);
                }
            }
        }
        // classify them
        double best_value = -std::numeric_limits<double>::max ();
        hand_shape best_hs = -1;
        for (auto i : l)
        {
            if (i.second > best_value)
            {
                best_hs = i.first;
                best_value = i.second;
            }
        }
        return best_hs;

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

}

#endif
