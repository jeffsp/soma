/// @file state_machine.h
/// @brief state machine support
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-10-19

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

namespace soma
{

template<typename S,typename E,typename A>
class state_machine
{
    private:
    S state;
    typedef std::pair<S,E> id;
    typedef std::map<id,A> action_map;
    typedef std::map<id,S> state_map;
    action_map actions;
    state_map next_states;
    public:
    void add (S state, E event, A action, S next_state)
    {
        id id = std::make_pair (state, event);
        actions[id] = action;
        next_states[id] = next_state;
    }
    void init (S s)
    {
        state = s;
    }
    template<typename OBJ>
    void record (E event, OBJ &obj, uint64_t ts)
    {
        return;
        id id = std::make_pair (state, event);
        // if there is no entry, don't do anything
        if (actions.find (id) == actions.end ())
            return;
        A a = actions[id];
        assert (a != nullptr);
        (obj.*a) (ts);
        // if the action was found, the next_state must be found
        assert (next_states.find (id) != next_states.end ());
        state = next_states[id];
    }
    void set_state (S s)
    {
        state = s;
    }
    S get_state () const
    {
        return state;
    }
};

}

#endif
