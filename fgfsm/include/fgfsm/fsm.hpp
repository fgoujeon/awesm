//Copyright Florian Goujeon 2021.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/fgfsm

#ifndef FGFSM_FSM_HPP
#define FGFSM_FSM_HPP

#include "fsm_configuration.hpp"
#include "state_transition_policy.hpp"
#include "internal_transition_policy.hpp"
#include "none.hpp"
#include "event.hpp"
#include "detail/for_each.hpp"
#include "detail/make_tuple.hpp"
#include "detail/transition_table_digest.hpp"
#include "detail/ignore_unused.hpp"
#include <queue>

namespace fgfsm
{

template<class TransitionTable, class Configuration = fsm_configuration>
class fsm
{
    private:
        using transition_table = TransitionTable;

        using state_transition_policy = typename Configuration::state_transition_policy;
        using internal_transition_policy = typename Configuration::internal_transition_policy;

        using transition_table_digest =
            detail::transition_table_digest<transition_table>
        ;
        using state_tuple  = typename transition_table_digest::state_tuple;
        using action_tuple = typename transition_table_digest::action_tuple;
        using guard_tuple  = typename transition_table_digest::guard_tuple;
        using event_tuple  = typename transition_table_digest::event_tuple;

    public:
        template<class Context>
        fsm(Context& context):
            states_(detail::make_tuple<state_tuple>(context)),
            actions_(detail::make_tuple<action_tuple>(context)),
            guards_(detail::make_tuple<guard_tuple>(context)),
            state_transition_policy_{context},
            internal_transition_policy_{context}
        {
        }

        //Check whether the given State is the active state type
        template<class State>
        bool is_active_state() const
        {
            constexpr auto given_state_index = detail::tlu::get_index
            <
                state_tuple,
                State
            >;
            return given_state_index == active_state_index_;
        }

        void process_event(const event_ref& evt)
        {
            if constexpr(Configuration::enable_event_queue)
            {
                //Defer event processing in case of recursive call
                if(processing_event_)
                {
                    deferred_events_.push(event{evt});
                    return;
                }

                struct processing_event_guard
                {
                    processing_event_guard(bool& b):
                        b(b)
                    {
                        b = true;
                    }

                    ~processing_event_guard()
                    {
                        b = false;
                    }

                    bool& b;
                };
                auto processing_guard = processing_event_guard{processing_event_};

                process_event_once(evt);

                //Process deferred event processings
                while(!deferred_events_.empty())
                {
                    process_event_once(deferred_events_.front());
                    deferred_events_.pop();
                }
            }
            else
            {
                process_event_once(evt);
            }
        }

    private:
        void process_event_once(const event_ref& evt)
        {
            process_event_in_active_state(evt);
            process_event_in_transition_table(evt);
        }

        /*
        Try and trigger a transition and potential subsequent anonymous
        transitions, if any.
        */
        void process_event_in_transition_table(const event_ref& evt)
        {
            const bool processed = process_event_in_transition_table_once(evt);
            detail::ignore_unused(processed);

            //Anonymous transitions
            if constexpr(detail::tlu::contains<event_tuple, none>)
                if(processed)
                    while(process_event_in_transition_table_once(none{}));
        }

        //Try and trigger one transition
        bool process_event_in_transition_table_once(const event_ref& evt)
        {
            bool processed = false;

            const auto helper = process_event_in_transition_table_once_helper
            {
                *this,
                evt,
                processed
            };

            //For each row
            detail::tlu::for_each<transition_table>(helper);

            return processed;
        }

        struct process_event_in_transition_table_once_helper
        {
            template<class Transition>
            void operator()() const
            {
                using transition_start_state  = typename Transition::start_state;
                using transition_event        = typename Transition::event;
                using transition_target_state = typename Transition::target_state;
                using transition_action       = typename Transition::action;
                using transition_guard        = typename Transition::guard;

                //Make sure we don't trigger more than one transition
                if(processed)
                    return;

                if(evt.is<transition_event>())
                {
                    //Make sure the transition start state is the active
                    //state
                    if(!self.is_active_state<transition_start_state>())
                        return;

                    auto& start_state =
                        std::get<transition_start_state>(self.states_)
                    ;
                    auto& target_state =
                        std::get<transition_target_state>(self.states_)
                    ;
                    auto& guard = std::get<transition_guard>(self.guards_);
                    auto& action = std::get<transition_action>(self.actions_);

                    const auto target_state_index = detail::tlu::get_index
                    <
                        state_tuple,
                        transition_target_state
                    >;

                    auto helper = state_transition_policy_helper
                    <
                        transition_start_state,
                        transition_target_state,
                        transition_action,
                        transition_guard
                    >
                    {
                        start_state,
                        evt,
                        target_state,
                        action,
                        guard,
                        self.active_state_index_,
                        processed,
                        target_state_index
                    };

                    //Perform the transition
                    self.state_transition_policy_(helper);
                }
            }

            fsm& self;
            const event_ref& evt;
            bool& processed;
        };

        /*
        Call active_state.on_event(event)
        */
        void process_event_in_active_state(const event_ref& evt)
        {
            detail::for_each
            (
                states_,
                [this, &evt](auto& s)
                {
                    using state = std::decay_t<decltype(s)>;
                    if(is_active_state<state>())
                    {
                        auto helper = internal_transition_policy_helper
                        <
                            state
                        >{s, evt};
                        internal_transition_policy_(helper);
                    }
                }
            );
        }

    private:
        state_tuple states_;
        action_tuple actions_;
        guard_tuple guards_;
        state_transition_policy state_transition_policy_;
        internal_transition_policy internal_transition_policy_;

        int active_state_index_ = 0;
        bool processing_event_ = false;
        std::queue<event> deferred_events_;
};

} //namespace

#endif
