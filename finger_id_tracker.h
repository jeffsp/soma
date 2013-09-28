/// @file finger_id_tracker.h
/// @brief finger_id_tracker class
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-23

#ifndef FINGER_ID_TRACKER_H
#define FINGER_ID_TRACKER_H

#include "soma.h"
#include <cstdint>
#include <map>
#include <vector>

namespace soma
{

/// @brief track finger ids over a sliding window
class finger_id_tracker
{
    public:
    /// @brief finger id type
    typedef std::vector<int32_t> finger_ids;
    private:
    /// @brief sliding window of ids
    sliding_window<finger_ids> w;
    /// @brief one vector of modes for each number of fingers
    std::map<size_t,std::vector<running_mode>> modes;
    /// @brief one vector of current ids for each number of fingers
    std::map<size_t,finger_ids> current_ids;
    /// @brief one vector of changed flags for each number of fingers
    std::map<size_t,bool> changed;
    public:
    /// @brief constructor
    ///
    /// @param duration sliding window duration
    finger_id_tracker (uint64_t duration)
        : w (duration)
    {
    }
    /// @brief add a vector of ids
    ///
    /// @param ts timestamp
    /// @param ids ids
    void add (uint64_t ts, const finger_ids &ids)
    {
        size_t nfingers = ids.size ();
        auto last = current_ids[nfingers];
        w.add (ts, ids, *this);
        changed[nfingers] = (last != current_ids[nfingers]);
    }
    /// @brief get the current ids
    ///
    /// @param nfingers number of fingers
    ///
    /// @return the ids
    finger_ids get_ids (size_t nfingers) const
    {
        auto i = current_ids.find (nfingers);
        if (i == current_ids.end ())
            return finger_ids (nfingers);
        else
            return i->second;
    }
    /// @brief have the ids changed since last add
    ///
    /// @param nfingers number fo fingers
    ///
    /// @return true if changed
    bool has_changed (size_t nfingers) const
    {
        auto i = changed.find (nfingers);
        if (i == changed.end ())
            return false;
        else
            return i->second;
    }
    /// @brief observer callback
    ///
    /// @param ids ids to add
    void add (const finger_ids &ids)
    {
        size_t nfingers = ids.size ();
        if (modes[nfingers].size () != nfingers)
            modes[nfingers].resize (nfingers);
        if (current_ids[nfingers].size () != nfingers)
            current_ids[nfingers].resize (nfingers);
        for (size_t i = 0; i < ids.size (); ++i)
        {
            modes[nfingers][i].add (ids[i]);
            current_ids[nfingers][i] = modes[nfingers][i].mode ();
        }
    }
    /// @brief observer callback
    ///
    /// @param ids ids to remove
    void remove (const finger_ids &ids)
    {
        size_t nfingers = ids.size ();
        for (size_t i = 0; i < ids.size (); ++i)
        {
            assert (i < modes[nfingers].size ());
            assert (i < current_ids[nfingers].size ());
            modes[nfingers][i].remove (ids[i]);
            current_ids[nfingers][i] = modes[nfingers][i].mode ();
        }
    }
};

}

#endif
