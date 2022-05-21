//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_INTERNAL_TRANSITION_POLICY_HELPER_HPP
#define AWESM_INTERNAL_TRANSITION_POLICY_HELPER_HPP

#include "detail/call_member.hpp"

namespace awesm
{

template<class State, class Event>
class internal_transition_policy_helper
{
    public:
        using state_type = State;
        using event_type = Event;

        void invoke_state_on_event()
        {
            state_.on_event(event_);
        }

    private:
        template<class FsmConfiguration>
        friend class fsm;

        internal_transition_policy_helper(State& state, const Event& event):
            state_(state),
            event_(event)
        {
        }

        State& state_;
        const Event& event_;
};

} //namespace

#endif