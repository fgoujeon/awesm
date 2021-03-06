//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_DETAIL_SM_CONFIGURATION_HPP
#define AWESM_DETAIL_SM_CONFIGURATION_HPP

#include "../sm_options.hpp"

namespace awesm::detail
{

template<class... Options>
class sm_configuration: private Options...
{
    public:
        template<class Sm, class Context>
        sm_configuration(Sm& mach, Context& ctx):
            Options(mach, ctx)...
        {
        }

        static constexpr bool has_in_state_internal_transitions()
        {
            return call
            (
                sm_options::detail::tags::in_state_internal_transitions{},
                0
            );
        }

        static constexpr bool has_run_to_completion()
        {
            return call
            (
                sm_options::detail::tags::run_to_completion{},
                0
            );
        }

        template<class Region, class SourceState, class Event, class TargetState>
        void after_state_transition(const Region& region, const Event& event)
        {
            call
            (
                sm_options::detail::tags::after_state_transition{},
                &region,
                static_cast<SourceState*>(nullptr),
                &event,
                static_cast<TargetState*>(nullptr),
                0
            );
        }

        template<class Region, class SourceState, class Event, class TargetState>
        void before_entry(const Region& region, const Event& event)
        {
            call
            (
                sm_options::detail::tags::before_entry{},
                &region,
                static_cast<SourceState*>(nullptr),
                &event,
                static_cast<TargetState*>(nullptr),
                0
            );
        }

        template<class Region, class SourceState, class Event, class TargetState>
        void before_state_transition(const Region& region, const Event& event)
        {
            call
            (
                sm_options::detail::tags::before_state_transition{},
                &region,
                static_cast<SourceState*>(nullptr),
                &event,
                static_cast<TargetState*>(nullptr),
                0
            );
        }

        template<class Event>
        auto on_event(const Event* pevent) ->
            decltype(call(sm_options::detail::tags::on_event{}, *pevent, 0))
        {
            call(sm_options::detail::tags::on_event{}, *pevent, 0);
        }

        void on_event(const void* /*pevent*/)
        {
        }

        void on_exception(const std::exception_ptr& eptr)
        {
            call(sm_options::detail::tags::on_exception{}, eptr, 0);
        }

    private:
        using Options::call...;
};

} //namespace

#endif
