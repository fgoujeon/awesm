//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_SM_OPTIONS_BEFORE_STATE_TRANSITION_HPP
#define AWESM_SM_OPTIONS_BEFORE_STATE_TRANSITION_HPP

#include "../detail/sm_object_holder.hpp"

namespace awesm::sm_options
{

namespace detail::tags
{
    struct before_state_transition{};
}

namespace detail::defaults
{
    struct before_state_transition
    {
        template<class Sm, class Context>
        before_state_transition(Sm& /*m*/, Context& /*ctx*/)
        {
        }

        void call
        (
            tags::before_state_transition /*action_tag*/,
            const void* /*pregion*/,
            void* /*source_state_tag*/,
            const void* /*pevent*/,
            void* /*target_state_tag*/,
            long /*priority*/
        )
        {
        }
    };
}

template<class T>
class before_state_transition
{
    public:
        template<class Sm, class Context>
        before_state_transition(Sm& mach, Context& ctx):
            impl_(mach, ctx)
        {
        }

        template<class Region, class SourceState, class Event, class TargetState>
        void call
        (
            detail::tags::before_state_transition /*action_tag*/,
            const Region* pregion,
            SourceState* /*source_state_tag*/,
            const Event* pevent,
            TargetState* /*target_state_tag*/,
            int /*priority*/
        )
        {
            impl_.object.template before_state_transition<Region, SourceState, Event, TargetState>
            (
                *pregion,
                *pevent
            );
        }

    private:
        awesm::detail::sm_object_holder<T> impl_;
};

} //namespace

#endif
