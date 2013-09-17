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

#include "audio.h"
#include "mouse.h"
#include "options.h"
#include "soma.h"
#include "stats.h"
#include <chrono>
#include <array>

namespace soma
{

// Euclidean 3D vector
struct vec3
{
    vec3 ()
        : x (0.0f)
        , y (0.0f)
        , z (0.0f)
    { }
    vec3 (const Leap::Vector &a)
        : x (a.x)
        , y (a.y)
        , z (a.z)
    { }
    vec3 (double x, double y, double z)
        : x (x)
        , y (y)
        , z (z)
    { }
    double x;
    double y;
    double z;
};

double distance (const vec3 &a, const vec3 &b)
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    double dz = a.z - b.z;
    return sqrt (dx * dx + dy * dy + dz * dz);
}

vec3 operator- (const vec3 &a, const vec3 &b)
{
    return vec3 (a.x - b.x, a.y - b.y, a.z - b.z);
}

bool operator== (const vec3 &a, const vec3 &b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool operator!= (const vec3 &a, const vec3 &b)
{
    return !(a == b);
}

/// @brief i/o helper
std::ostream& operator<< (std::ostream &s, const vec3 &p)
{
    s << p.x << ' ' << p.y << ' ' << p.z;
    return s;
}

typedef std::vector<vec3> vec3s;

vec3s get_positions (const Leap::PointableList &l)
{
    vec3s p (l.count ());
    for (size_t i = 0; i < p.size (); ++i)
        p[i] = l[i].tipPosition ();
    return p;
}

vec3s get_velocities (const Leap::PointableList &l)
{
    vec3s p (l.count ());
    for (size_t i = 0; i < p.size (); ++i)
        p[i] = l[i].tipVelocity ();
    return p;
}

vec3s get_directions (const Leap::PointableList &l)
{
    vec3s p (l.count ());
    for (size_t i = 0; i < p.size (); ++i)
        p[i] = l[i].direction ();
    return p;
}

/// @brief i/o helper
std::ostream& operator<< (std::ostream &s, const vec3s &p)
{
    for (auto i : p)
        s << ' ' << i;
    return s;
}

const uint64_t FINGER_COUNTER_WINDOW_DURATION = 200000;
const double FINGER_COUNTER_WINDOW_FULLNESS = 0.85f;
const double FINGER_COUNTER_CERTAINTY = 0.8f;

class finger_counter
{
    private:
    sliding_time_window<unsigned> w;
    unsigned current_count;
    public:
    finger_counter ()
        : w (FINGER_COUNTER_WINDOW_DURATION)
        , current_count (~0)
    {
    }
    void update (uint64_t ts, const unsigned count)
    {
        w.add_sample (ts, count);
        // if it's mostly full
        if (w.fullness (ts) > FINGER_COUNTER_WINDOW_FULLNESS)
        {
            // get count
            auto s = w.get_samples ();
            assert (s.size () > 0);
            unsigned count = mode (s);
            double total = 0.0f;
            for (auto i : s)
                if (i == count)
                    ++total;
            double certainty = total / s.size ();
            // don't change unless certainty is high
            if (certainty < FINGER_COUNTER_CERTAINTY)
                return;
            current_count = count;
        }
    }
    unsigned get_count () const
    {
        return current_count;
    }
};

class position_tracker
{
    private:
    vec3s current_positions;
    public:
    void update (const unsigned count, const vec3s &p)
    {
        // simply filter out samples that don't have the correct current size
        if (count == p.size ())
            current_positions = p;
    }
    const vec3s &get_positions () const
    {
        return current_positions;
    }
};

enum class input_mode { zero, point, click, scroll, center };

std::string to_string (const input_mode m)
{
    switch (m)
    {
        default: assert (0); return std::string ("unknown");
        case input_mode::zero: return std::string ("zero");
        case input_mode::point: return std::string ("point");
        case input_mode::click: return std::string ("click");
        case input_mode::scroll: return std::string ("scroll");
        case input_mode::center: return std::string ("center");
    }
}

class mode_switcher
{
    private:
    input_mode current;
    public:
    mode_switcher ()
        : current (input_mode::zero)
    {
    }
    void update (const vec3s &p)
    {
        switch (p.size ())
        {
            default:
            current = input_mode::zero;
            break;
            case 1:
            current = input_mode::point;
            break;
            case 2:
            current = input_mode::scroll;
            break;
            case 5:
            current = input_mode::center;
            break;
        }
    }
    input_mode get_mode () const
    {
        return current;
    }
};

class mouse_pointer
{
    private:
    mouse &m;
    vec3 last_point;
    bool last_point_valid;
    double speed;
    public:
    mouse_pointer (mouse &m, double speed)
        : m (m)
        , last_point_valid (false)
        , speed (speed)
    {
    }
    void set_speed (double s)
    {
        if (s >= 1.0)
            speed = s;
    }
    void clear ()
    {
        last_point_valid = false;
    }
    void update (const vec3s &p)
    {
        assert (!p.empty ());
        if (last_point_valid)
        {
            double x = last_point.x - p[0].x;
            double y = last_point.y - p[0].y;
            m.move (-x * speed, y * speed);
        }
        last_point = p[0];
        last_point_valid = true;
    }
    void center ()
    {
        m.center ();
    }
};

class mouse_clicker
{
    private:
    mouse &m;
    public:
    mouse_clicker (mouse &m)
        : m (m)
    {
    }
    void clear ()
    {
    }
    void update (const vec3s &p)
    {
        assert (!p.empty ());
    }
};

class mouse_scroller
{
    private:
    mouse &m;
    public:
    mouse_scroller (mouse &m)
        : m (m)
    {
    }
    void clear ()
    {
    }
    void update (const vec3s &p)
    {
        assert (!p.empty ());
    }
};

void beep (const audio &au, const input_mode &m)
{
    const int D = 300; // millisecs
    switch (m)
    {
        default: assert (0); break;
        case input_mode::zero: au.play (98, D); break;
        case input_mode::point: au.play (262, D); break;
        case input_mode::click: au.play (392, D); break;
        case input_mode::scroll: au.play (523, D); break;
        case input_mode::center:
        au.play (262, D);
        au.play (1, D);
        au.play (262, D);
        au.play (1, D);
        au.play (262, D);
        break;
    }
}

class soma_mouse : public Leap::Listener
{
    private:
    bool done;
    const options &opts;
    frame_counter frc;
    finger_counter fic;
    position_tracker pt;
    mode_switcher ms;
    audio au;
    mouse m;
    mouse_pointer mop;
    mouse_clicker moc;
    mouse_scroller mos;
    public:
    soma_mouse (const options &opts)
        : done (false)
        , opts (opts)
        , mop (m, opts.get_mouse_speed ())
        , moc (m)
        , mos (m)
    {
    }
    ~soma_mouse ()
    {
        std::clog << frc.fps () << "fps" << std::endl;
    }
    bool is_done () const
    {
        return done;
    }
    virtual void onInit (const Leap::Controller&)
    {
        std::clog << "onInit()" << std::endl;
    }
    virtual void onConnect (const Leap::Controller&)
    {
        std::clog << "onConnect()" << std::endl;
    }
    virtual void onDisconnect (const Leap::Controller&)
    {
        std::clog << "onDisconnect()" << std::endl;
    }
    virtual void onFrame(const Leap::Controller& c)
    {
        if (done)
            return;
        const Leap::Frame &f = c.frame ();
        frc.update (f.timestamp ());
        const vec3s &p = get_positions (f.pointables ());
        fic.update (f.timestamp (), p.size ());
        pt.update (fic.get_count (), p);
        ms.update (pt.get_positions ());
        switch (ms.get_mode ())
        {
            default:
                assert (0);
            break;
            case input_mode::zero:
            mop.clear ();
            moc.clear ();
            mos.clear ();
            break;
            case input_mode::point:
            mop.update (pt.get_positions ());
            moc.clear ();
            mos.clear ();
            break;
            case input_mode::click:
            mop.clear ();
            moc.update (pt.get_positions ());
            mos.clear ();
            break;
            case input_mode::scroll:
            mop.clear ();
            moc.clear ();
            mos.update (pt.get_positions ());
            break;
            case input_mode::center:
            mop.clear ();
            mop.center ();
            moc.clear ();
            mos.clear ();
            break;
        }
    }
};

}

#endif
