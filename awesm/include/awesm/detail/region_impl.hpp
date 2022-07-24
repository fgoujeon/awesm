//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_DETAIL_REGION_IMPL_HPP
#define AWESM_DETAIL_REGION_IMPL_HPP

#include "call_member.hpp"
#include "resolve_transition_table.hpp"
#include "transition_table_digest.hpp"
#include "alternative_lazy.hpp"
#include "any_container.hpp"
#include "ignore_unused.hpp"
#include "event_processing_type.hpp"
#include "../none.hpp"
#include "../null_state.hpp"
#include "tlu/apply.hpp"
#include <type_traits>

namespace awesm::detail
{

template<class Derived, class TransitionTable>
class region_impl
{
    public:
        template<class Sm, class Context>
        explicit region_impl(Sm& mach, Context& context):
            states_(mach, context),
            actions_(mach, context),
            guards_(mach, context)
        {
        }

        region_impl(const region_impl&) = delete;
        region_impl(region_impl&&) = delete;
        region_impl& operator=(const region_impl&) = delete;
        region_impl& operator=(region_impl&&) = delete;
        ~region_impl() = default;

        //Check whether the given State is the active state type
        template<class State>
        [[nodiscard]] bool is_active_state() const
        {
            constexpr auto given_state_index = detail::tlu::get_index
            <
                state_tuple_t,
                State
            >;
            return active_state_index_ == given_state_index;
        }

        template<class State>
        const auto& get_state() const
        {
            return states_.get(static_cast<State*>(nullptr));
        }

        template<class SmConfiguration, class Event = none>
        void start(SmConfiguration& sm_conf, const Event& event = {})
        {
            process_event_2<detail::event_processing_type::start>(sm_conf, event);
        }

        template<class SmConfiguration, class Event = none>
        void stop(SmConfiguration& sm_conf, const Event& event = {})
        {
            process_event_2<detail::event_processing_type::stop>(sm_conf, event);
        }

        template<class SmConfiguration, class Event>
        void process_event(SmConfiguration& sm_conf, const Event& event)
        {
            process_event_2<detail::event_processing_type::event>(sm_conf, event);
        }

    private:
        using unresolved_transition_table_t = TransitionTable;

        using transition_table_digest_t =
            detail::transition_table_digest<unresolved_transition_table_t>
        ;
        using state_tuple_t = typename transition_table_digest_t::state_tuple;
        using action_tuple_t = typename transition_table_digest_t::action_tuple;
        using guard_tuple_t = typename transition_table_digest_t::guard_tuple;

        using initial_state_t = detail::tlu::at<state_tuple_t, 1>; //0 being null_state

        /*
        Calling detail::resolve_transition_table<> isn't free. We need
        alternative_lazy to avoid the call when it's unnecessary (i.e. when
        there's no pattern-source-state in the transition table).
        */
        struct unresolved_transition_table_holder
        {
            template<class = void>
            using type = unresolved_transition_table_t;
        };
        struct resolved_transition_table_holder
        {
            template<class = void>
            using type = detail::resolve_transition_table
            <
                unresolved_transition_table_t,
                state_tuple_t
            >;
        };
        using transition_table_t = detail::alternative_lazy
        <
            transition_table_digest_t::has_source_state_patterns,
            resolved_transition_table_holder,
            unresolved_transition_table_holder
        >;

        template
        <
            detail::event_processing_type ProcessingType,
            class SmConfiguration,
            class Event
        >
        void process_event_2(SmConfiguration& sm_conf, const Event& event)
        {
            if constexpr(SmConfiguration::has_in_state_internal_transitions())
            {
                process_event_in_active_state(sm_conf, event);
            }

            auto processed = true;
            if constexpr(ProcessingType == detail::event_processing_type::start)
            {
                using fake_row = row<null_state, Event, initial_state_t>;
                processed = transition_table_row_event_processor<fake_row>::process
                (
                    *this,
                    &sm_conf,
                    &event
                );
            }
            else if constexpr(ProcessingType == detail::event_processing_type::stop)
            {
                processed = detail::tlu::apply
                <
                    state_tuple_t,
                    stop_helper
                >::call(*this, sm_conf, event);
            }
            else if constexpr(ProcessingType == detail::event_processing_type::event)
            {
                processed = process_event_in_transition_table_once(sm_conf, event);
            }

            //Anonymous transitions
            if constexpr(transition_table_digest_t::has_none_events)
            {
                if(processed)
                {
                    while(process_event_in_transition_table_once(sm_conf, none{})){}
                }
            }
            else
            {
                detail::ignore_unused(processed);
            }
        }

        //Used to call client code
        template<class SmConfiguration, class F>
        void safe_call(SmConfiguration& sm_conf, F&& fun)
        {
            try
            {
                fun();
            }
            catch(...)
            {
                sm_conf.on_exception(std::current_exception());
            }
        }

        //Try and trigger one transition
        template<class SmConfiguration, class Event>
        bool process_event_in_transition_table_once(SmConfiguration& sm_conf, const Event& event)
        {
            return detail::tlu::apply
            <
                transition_table_t,
                transition_table_event_processor
            >::process(*this, sm_conf, event);
        }

        template<class... Rows>
        struct transition_table_event_processor
        {
            template<class SmConfiguration, class Event>
            static bool process(region_impl& reg, SmConfiguration& sm_conf, const Event& event)
            {
                return
                (
                    transition_table_row_event_processor<Rows>::process
                    (
                        reg,
                        &sm_conf,
                        &event
                    ) || ...
                );
            }
        };

        //Processes events against one row of the (resolved) transition table
        template<class Row>
        struct transition_table_row_event_processor
        {
            template<class SmConfiguration>
            static bool process
            (
                region_impl& reg,
                SmConfiguration* psm_conf,
                const typename Row::event_type* const pevent
            )
            {
                return reg.process_event_in_row<Row>(*psm_conf, *pevent);
            }

            /*
            This void* trick allows for shorter build times because we save a
            template instantiation.
            Using an if-constexpr in the process() function above is worse as
            well.
            */
            static bool process(region_impl& /*reg*/, void* /*psm_conf*/, const void* /*pevent*/)
            {
                return false;
            }
        };

        template<class Row, class SmConfiguration, class Event>
        bool process_event_in_row(SmConfiguration& sm_conf, const Event& event)
        {
            using source_state_t = typename Row::source_state_type;
            using target_state_t = typename Row::target_state_type;
            using action_t       = typename Row::action_type;
            using guard_t        = typename Row::guard_type;

            //Make sure the transition source state is the active state
            if(!is_active_state<source_state_t>())
            {
                return false;
            }

            const auto do_transition = [&](auto /*dummy*/)
            {
                constexpr auto is_internal_transition =
                    std::is_same_v<target_state_t, none>
                ;

                if constexpr(!is_internal_transition)
                {
                    sm_conf.template before_state_transition
                    <
                        Derived,
                        source_state_t,
                        Event,
                        target_state_t
                    >(cself(), event);

                    detail::call_on_exit
                    (
                        &states_.get(static_cast<source_state_t*>(nullptr)),
                        &sm_conf,
                        &event,
                        0
                    );

                    active_state_index_ = detail::tlu::get_index
                    <
                        state_tuple_t,
                        target_state_t
                    >;
                }

                if constexpr(!std::is_same_v<action_t, none>)
                {
                    detail::call_execute
                    (
                        &actions_.get(static_cast<action_t*>(nullptr)),
                        &event
                    );
                }

                if constexpr(!is_internal_transition)
                {
                    sm_conf.template before_entry
                    <
                        Derived,
                        source_state_t,
                        Event,
                        target_state_t
                    >(cself(), event);

                    detail::call_on_entry
                    (
                        &states_.get(static_cast<target_state_t*>(nullptr)),
                        &sm_conf,
                        &event,
                        0
                    );

                    sm_conf.template after_state_transition
                    <
                        Derived,
                        source_state_t,
                        Event,
                        target_state_t
                    >(cself(), event);
                }
            };

            if constexpr(std::is_same_v<guard_t, none>)
            {
                safe_call
                (
                    sm_conf,
                    [&]
                    {
                        do_transition(0);
                    }
                );
                return true;
            }
            else
            {
                auto processed = false;
                safe_call
                (
                    sm_conf,
                    [&]
                    {
                        if
                        (
                            !detail::call_check
                            (
                                &guards_.get(static_cast<guard_t*>(nullptr)),
                                &event
                            )
                        )
                        {
                            return;
                        }

                        processed = true;
                        do_transition(0);
                    }
                );
                return processed;
            }
        }

        template<class... States>
        struct stop_helper
        {
            template<class SmConfiguration, class Event>
            static bool call(region_impl& reg, SmConfiguration& sm_conf, const Event& event)
            {
                return (reg.stop_2<States>(sm_conf, &event) || ...);
            }
        };

        template<class State, class SmConfiguration, class Event>
        bool stop_2(SmConfiguration& sm_conf, const Event* pevent)
        {
            using fake_row = row<State, Event, null_state>;
            return transition_table_row_event_processor<fake_row>::process
            (
                *this,
                &sm_conf,
                pevent
            );
        }

        /*
        Call active_state.on_event(event)
        */
        template<class SmConfiguration, class Event>
        void process_event_in_active_state(SmConfiguration& sm_conf, const Event& event)
        {
            return detail::tlu::apply
            <
                state_tuple_t,
                active_state_event_processor
            >::process(*this, sm_conf, event);
        }

        //Processes internal events against all states
        template<class... States>
        struct active_state_event_processor
        {
            template<class SmConfiguration, class Event>
            static void process(region_impl& reg, SmConfiguration& sm_conf, const Event& event)
            {
                (reg.process_event_in_state<States>(&sm_conf, &event) || ...);
            }
        };

        //Processes internal events against one state
        template<class State, class SmConfiguration, class Event>
        auto process_event_in_state
        (
            SmConfiguration* psm_conf,
            const Event* pevent
        ) -> decltype(std::declval<State>().on_event(*psm_conf, *pevent), bool())
        {
            if(is_active_state<State>())
            {
                safe_call
                (
                    *psm_conf,
                    [&]
                    {
                        states_.get(static_cast<State*>(nullptr)).on_event(*psm_conf, *pevent);
                    }
                );
                return true;
            }
            return false;
        }

        //Processes internal events against one state
        template<class State, class SmConfiguration, class Event>
        auto process_event_in_state
        (
            SmConfiguration* psm_conf,
            const Event* pevent
        ) -> decltype(std::declval<State>().on_event(*pevent), bool())
        {
            if(is_active_state<State>())
            {
                safe_call
                (
                    *psm_conf,
                    [&]
                    {
                        states_.get(static_cast<State*>(nullptr)).on_event(*pevent);
                    }
                );
                return true;
            }
            return false;
        }

        template<class State>
        bool process_event_in_state
        (
            void* /*psm_conf*/,
            const void* /*pevent*/
        )
        {
            return false;
        }

        const Derived& cself() const
        {
            return static_cast<const Derived&>(*this);
        }

        state_tuple_t states_;
        action_tuple_t actions_;
        guard_tuple_t guards_;

        int active_state_index_ = 0;
};

} //namespace

#endif
