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
#include "Leap.h"

namespace soma
{

/// @brief keep track of frames and the time it takes to display them
class frame_counter
{
    private:
    uint64_t frames;
    uint64_t first_ts;
    uint64_t last_ts;
    public:
    frame_counter ()
        : frames (0)
        , first_ts (0)
        , last_ts (0)
    {
    }
    /// @brief update the counter
    ///
    /// @param ts timestamp of the frame
    void update (uint64_t ts)
    {
        if (frames == 0)
            first_ts = ts;
        else
            last_ts = ts;
        ++frames;
    }
    /// @brief get number of frames counted
    ///
    /// @return total frames counted
    uint64_t get_frames () const
    {
        return frames;
    }
    /// @brief get frames per second
    ///
    /// @return fps
    double fps () const
    {
        double secs = static_cast<double> (last_ts - first_ts) / 1000000;
        // don't count the last frame
        if (frames != 0 && secs != 0.0f)
            return (frames - 1) / secs;
        return -1;
    }
};

/// @brief A sliding window of samples
///
/// @tparam T sample type
template<typename T>
class sliding_time_window
{
    private:
    const uint64_t duration;
    std::deque<uint64_t> timestamps;
    std::deque<T> samples;
    void update (uint64_t ts)
    {
        while (!timestamps.empty ())
        {
            assert (samples.size () == timestamps.size ());
            assert (ts >= timestamps.back ());
            if (ts - timestamps.back () > duration)
            {
                timestamps.pop_back ();
                samples.pop_back ();
            }
            else
                break;
        }
    }
    public:
    /// @brief constructor
    ///
    /// @param duration duration of the window in useconds
    sliding_time_window (uint64_t duration)
        : duration (duration)
    {
    }
    /// @brief remove all samples from the window
    void clear ()
    {
        samples.clear ();
        timestamps.clear ();
    }
    /// @brief get the fullness at a given time
    ///
    /// @param ts the time
    ///
    /// @return 0.0, 1.0 where 1.0 is full and 0.0 is empty
    double fullness (uint64_t ts) const
    {
        if (timestamps.empty ())
            return 0.0f;
        double start = timestamps.back ();
        assert (start <= ts);
        assert (duration != 0);
        return (ts - start) / duration;
    }
    /// @brief add a sample
    ///
    /// @param ts timestamp in useconds
    /// @param n the sample
    void add_sample (uint64_t ts, const T &n)
    {
        // remove samples with old timestamps
        update (ts);
        // don't add the same sample twice
        assert (timestamps.empty () || timestamps.front () != ts);
        timestamps.emplace_front (ts);
        samples.emplace_front (n);
    }
    /// @brief get a reference to the samples
    ///
    /// @return the samples
    const std::deque<T> get_samples () const
    {
        return samples;
    }
    /// @brief get a rerence to the timestamps
    ///
    /// @return the timestamps
    const std::deque<uint64_t> get_timestamps () const
    {
        return timestamps;
    }
    /// @brief dump the samples to a stream
    ///
    /// @param s the stream
    void dump (std::ostream &s) const
    {
        uint64_t start = samples.front ().first;
        for (auto i : samples)
            s << ' ' << '<' << start - i.first << ',' << i.second << '>';
        s << std::endl;
    }
};

struct finger_sample
{
    int32_t id;
    Leap::Vector position;
    Leap::Vector velocity;
    Leap::Vector direction;
    finger_sample ()
        : id (std::numeric_limits<uint32_t>::max ())
    {
    }
};

bool sort_by_id (const finger_sample &a, const finger_sample &b)
{
    return a.id < b.id;
}

bool sort_left_to_right (const finger_sample &a, const finger_sample &b)
{
    return a.position.x < b.position.x;
}

class hand_sample : public std::vector<finger_sample>
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

class hand_shape_feature_vector : public std::vector<double>
{
    public:
    hand_shape_feature_vector (const hand_sample &h)
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
                Leap::Vector dir = h[i].position - h[i + 1].position;
                push_back (dir.x);
                push_back (dir.y);
                push_back (dir.z);
            }
        }
    }
    static size_t dimensions (size_t fingers)
    {
        assert (fingers > 0);
        return 1 + fingers * 3 + (fingers - 1) * 4;
    }
};

typedef std::vector<uint64_t> timestamps;
typedef std::vector<hand_shape_feature_vector> hand_shape_feature_vectors;

enum class hand_shape
{
    unknown,
    pointing,
    clicking,
    scrolling,
    centering
};

std::string to_string (const hand_shape hs)
{
    switch (hs)
    {
        default:
        throw std::runtime_error ("invalid hand position");
        case hand_shape::unknown:
        return std::string ("unknown");
        case hand_shape::pointing:
        return std::string ("pointing");
        case hand_shape::clicking:
        return std::string ("clicking");
        case hand_shape::scrolling:
        return std::string ("scrolling");
        case hand_shape::centering:
        return std::string ("centering");
    }
}

class dist_vectors
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
    friend std::ostream& operator<< (std::ostream &s, const dist_vectors &x)
    {
        s << x.total << std::endl;
        assert (x.u1.size () == x.u2.size ());
        s << x.u1.size () << std::endl;
        s.precision (12);
        for (size_t i = 0; i < x.u1.size (); ++i)
            s << ' ' << x.u1[i] << ' ' << x.u2[i];
        return s;
    }
    friend std::istream& operator>> (std::istream &s, dist_vectors &x)
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

const std::vector<hand_shape> hand_shapes { hand_shape::pointing, hand_shape::clicking, hand_shape::scrolling, hand_shape::centering };

class hand_shape_classifier
{
    private:
    typedef std::map<hand_shape,dist_vectors> map_hand_shape_dists;
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
                hss[i][hs].resize (hand_shape_feature_vector::dimensions (i + 1));
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
    void classify (const hand_shape_feature_vectors &hsfvs, const timestamps &ts, std::map<hand_shape,double> &l) const
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
                dist_vectors x;
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
