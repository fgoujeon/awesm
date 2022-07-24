//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_COMPOSITE_STATE_HPP
#define AWESM_COMPOSITE_STATE_HPP

#include "detail/region_tuple.hpp"
#include "detail/sm_object_holder.hpp"

namespace awesm
{

template<class Definition, class RegionList>
class composite_state
{
    public:
        template<class Sm, class Context>
        composite_state(Sm& mach, Context& ctx):
            def_(mach, ctx),
            region_tuple_(mach, ctx)
        {
        }

        template<class State, int RegionIndex = 0>
        [[nodiscard]] bool is_active_state() const
        {
            return region_tuple_.template is_active_state<State, RegionIndex>();
        }

        template<class SmConfiguration, class Event>
        void on_entry(SmConfiguration& sm_conf, const Event& event)
        {
            def_.object.on_entry(event);
            region_tuple_.start(sm_conf, event);
        }

        template<class SmConfiguration, class Event>
        void on_event(SmConfiguration& sm_conf, const Event& event)
        {
            region_tuple_.process_event(sm_conf, event);
            def_.object.on_event(event);
        }

        template<class SmConfiguration, class Event>
        void on_exit(SmConfiguration& sm_conf, const Event& event)
        {
            region_tuple_.stop(sm_conf, event);
            def_.object.on_exit(event);
        }

    private:
        detail::sm_object_holder<Definition> def_;
        detail::region_tuple<RegionList> region_tuple_;
};

} //namespace

#endif
