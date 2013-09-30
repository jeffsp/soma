/// @file soma_mouse.h
/// @brief soma mouse implementation
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-05

#ifndef SOMA_MOUSE_H
#define SOMA_MOUSE_H

/// @brief version info
const int MAJOR_REVISION = 0;
const int MINOR_REVISION = 1;

#include "mouse.h"
#include "options.h"
#include "soma.h"

namespace soma
{

class mouse_pointer
{
    private:
    static const uint64_t SW_DURATION = 50000;
    sliding_window<int> swx;
    sliding_window<int> swy;
    running_mean x;
    running_mean y;
    mouse &m;
    double speed;
    bool last_valid;
    double last_x;
    double last_y;
    public:
    mouse_pointer (mouse &m, double speed)
        : swx (SW_DURATION)
        , swy (SW_DURATION)
        , m (m)
        , speed (speed)
        , last_valid (false)
    {
    }
    void set_speed (double s)
    {
        if (s >= 1.0)
            speed = s;
    }
    void clear ()
    {
        swx.clear ();
        swy.clear ();
        x.reset ();
        y.reset ();
        last_valid = false;
    }
    void update (uint64_t ts, const vec3 &pos)
    {
        swx.add (ts, pos.x, x);
        swy.add (ts, pos.y, y);
        if (last_valid)
        {
            double px = last_x - x.mean ();
            double py = last_y - y.mean ();
            m.move (-px * speed, py * speed);
        }
        last_valid = true;
        last_x = x.mean ();
        last_y = y.mean ();
    }
    void center ()
    {
        m.center ();
    }
};

class soma_mouse : public Leap::Listener
{
    private:
    bool done;
    const options &opts;
    const hand_shape_classifier &hsc;
    static const uint64_t SWHS_DURATION = 100000;
    sliding_window<hand_sample> swhs;
    mouse_pointer mp;
    frame_counter fc;
    mouse m;
    timestamps ts;
    void update (uint64_t ts, const hand_shape shape, const hand_sample &hs)
    {
        switch (shape)
        {
            default:
            assert (0);
            break;
            case -1:
            mp.clear ();
            break;
            case 0:
            case 1:
            {
                if (!hs.empty ())
                {
                    hand_sample tmp (hs);
                    sort (tmp.begin (), tmp.end (), sort_top_to_bottom);
                    mp.update (ts, tmp[0].position);
                }
            }
            break;
            case 2:
            mp.clear ();
            break;
            case 3:
            mp.clear ();
            break;
            case 4:
            mp.clear ();
            mp.center ();
            break;
        }
    }
    public:
    soma_mouse (const options &opts, const hand_shape_classifier &hsc)
        : done (false)
        , opts (opts)
        , hsc (hsc)
        , swhs (SWHS_DURATION)
        , mp (m, opts.get_mouse_speed ())
    {
    }
    ~soma_mouse ()
    {
        std::clog << fc.fps () << "fps" << std::endl;
    }
    bool is_done () const
    {
        return done;
    }
    virtual void onFrame (const Leap::Controller& c)
    {
        if (done)
            return;
        // get the frame
        Leap::Frame f = c.frame ();
        uint64_t ts = f.timestamp ();
        // get the sample
        hand_sample hs (f.pointables ());
        // add it to the window
        swhs.add (ts, hs);
        // update frame counter
        fc.update (ts);
        // TODO Start
        // update finger counter
        // update finger id tracker
        // update sw of hand samples
        // update sw of feature vectors
        // determine if we need to get hand shape classification
        //    - new hand shape classification should be run every 50ms or so
        //    - if yes, run through samples and determine if more than 50% have same # of fingers
        //    - if yes, run through samples and determine if more than 50% have same finegr ids
        //    - if it all checks out, get new hand shape
        // update mouse pointer
        // TODO End
        hand_samples s;
        for (auto i : swhs.get_samples ())
            s.push_back (i);
        // did we get anything?
        if (s.empty ())
        {
            update (ts, -1, hs);
            return;
        }
        // filter out bad samples
        //hand_samples fs = filter (s);
        //const size_t nf = s.size () - fs.size ();
        // make sure they are reliable samples
        //if (100 * nf / s.size () > 25) // more than 25%?
        //{
        //    update (ts, -1, hs);
        //    return;
        //}
        // end if you show 6 or more fingers
        if (!s.empty () && s[0].size () > 5)
        {
            done = true;
            return;
        }
        // convert them to feature vectors
        std::vector<hand_shape_features> fv (s.begin (), s.end ());
        // classify them
        std::map<hand_shape,double> l;
        hand_shape shape = hsc.classify (fv.begin (), fv.end (), l);
        update (ts, shape, hs);
    }
};

}

#endif
