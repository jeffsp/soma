/// @file finger_id_tracker.h
/// @brief finger_id_tracker class
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-23

#ifndef FINGER_ID_TRACKER_H
#define FINGER_ID_TRACKER_H

#include "soma.h"
#include <cstdint>
#include <vector>

namespace soma
{

/// @brief track finger ids over a sliding window
class finger_id_tracker
{
    public:
    typedef std::vector<int32_t> finger_ids;
    private:
    std::vector<running_mode> modes;
    finger_ids current_ids;
    sliding_window<finger_ids> w;
    bool changed;
    public:
    finger_id_tracker (uint64_t duration)
        : w (duration)
        , changed (false)
    {
    }
    void add (uint64_t ts, const finger_ids &ids)
    {
        finger_ids last = current_ids;
        w.add (ts, ids, *this);
        changed = (last != current_ids);
    }
    finger_ids get_ids () const
    {
        return current_ids;
    }
    bool has_changed () const
    {
        return changed;
    }
    /// @brief observer callback
    ///
    /// @param ids ids to add
    void add (const finger_ids &ids)
    {
        // if they don't have the same number of fingers, start the running modes over
        if (ids.size () != modes.size ())
        {
            modes.clear ();
            modes.resize (ids.size ());
            current_ids.resize (ids.size ());
        }
        assert (modes.size () == ids.size ());
        assert (modes.size () == current_ids.size ());
        for (size_t i = 0; i < ids.size (); ++i)
        {
            modes[i].add (ids[i]);
            current_ids[i] = modes[i].mode ();
        }
    }
    /// @brief observer callback
    ///
    /// @param ids ids to remove
    void remove (const finger_ids &ids)
    {
        assert (modes.size () == ids.size ());
        assert (modes.size () == current_ids.size ());
        for (size_t i = 0; i < ids.size (); ++i)
        {
            modes[i].remove (ids[i]);
            current_ids[i] = modes[i].mode ();
        }
    }
};

}

#endif
