//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_DETAIL_SUBSM_HPP
#define AWESM_DETAIL_SUBSM_HPP

#include "../region.hpp"
#include "../region_list.hpp"
#include <type_traits>

namespace awesm::detail
{

template<class RegionList>
class subsm;

template<class... Regions>
class subsm<region_list<Regions...>>
{
    public:
        template<class Sm, class Context>
        explicit subsm(Sm& top_level_sm, Context& context):
            regions_{top_level_sm, context}
        {
        }

        subsm(const subsm&) = delete;
        subsm(subsm&&) = delete;
        subsm& operator=(const subsm&) = delete;
        subsm& operator=(subsm&&) = delete;
        ~subsm() = default;

        template<int RegionIndex>
        const auto& get_region() const
        {
            return regions_.template get<RegionIndex>();
        }

        template<class State, int RegionIndex>
        const auto& get_state() const
        {
            return get_region<RegionIndex>().template get_state<State>();
        }

        template<class State, int RegionIndex>
        [[nodiscard]] bool is_active_state() const
        {
            return regions_.template get<RegionIndex>().template is_active_state<State>();
        }

        template<class SmConfiguration, class Event = none>
        void start(SmConfiguration& sm_conf, const Event& event = {})
        {
            regions_.for_each
            (
                [&](auto& reg)
                {
                    reg.start(sm_conf, event);
                }
            );
        }

        template<class SmConfiguration, class Event = none>
        void stop(SmConfiguration& sm_conf, const Event& event = {})
        {
            regions_.for_each
            (
                [&](auto& reg)
                {
                    reg.stop(sm_conf, event);
                }
            );
        }

        template<class SmConfiguration, class Event>
        void process_event(SmConfiguration& sm_conf, const Event& event)
        {
            regions_.for_each
            (
                [&](auto& reg)
                {
                    reg.process_event(sm_conf, event);
                }
            );
        }

    private:
        sm_object_holder_tuple<Regions...> regions_;
};

} //namespace

#endif
