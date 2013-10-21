/// @file state_machine.h
/// @brief state machine support
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-19

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

namespace soma
{

/// @brief state machine
///
/// @tparam S state type
/// @tparam E event type
/// @tparam A action type
template<typename S,typename E,typename A>
class state_machine
{
    private:
    /// @brief unique id type
    typedef std::pair<S,E> id;
    /// @brief map an id to an action type
    typedef std::map<id,A> action_map;
    /// @brief map an id to the next state type
    typedef std::map<id,S> state_map;
    /// @brief current state
    S state;
    /// @brief map an id to an action
    action_map actions;
    /// @brief map an id to the next state
    state_map next_states;
    public:
    /// @brief initialize the action/next_state maps
    ///
    /// @param state current state
    /// @param event received event
    /// @param action action to take for this state/event pair
    /// @param next_state next state for this state/event pair
    void add (S state, E event, A action, S next_state)
    {
        id id = std::make_pair (state, event);
        actions[id] = action;
        next_states[id] = next_state;
    }
    /// @brief record an event
    ///
    /// @tparam OBJ object action type
    /// @param event event
    /// @param obj object upon which to invoke action
    /// @param ts timestamp
    template<typename OBJ>
    void record (E event, OBJ &obj, uint64_t ts)
    {
        std::clog << "state: " << int (state);
        std::clog << "\tevent: " << int (event);
        id id = std::make_pair (state, event);
        // if there is no entry, don't do anything
        if (actions.find (id) == actions.end ())
        {
            std::clog << "\tno action" << std::endl;
            return;
        }
        A a = actions[id];
        assert (a != nullptr);
        (obj.*a) (ts);
        // if the action was found, the next_state must be found
        assert (next_states.find (id) != next_states.end ());
        state = next_states[id];
        std::clog << "\tstate: " << int (state) << std::endl;
    }
    /// @brief set the state
    ///
    /// @param s state
    void set_state (S s)
    {
        state = s;
    }
    /// @brief get the current state
    ///
    /// @return current state
    S get_state () const
    {
        return state;
    }
};

}

#endif
