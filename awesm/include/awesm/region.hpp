//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_REGION_HPP
#define AWESM_REGION_HPP

#include "detail/region_impl.hpp"

namespace awesm
{

template<class TransitionTable>
class region:
    private detail::region_impl
    <
        region<TransitionTable>,
        TransitionTable
    >
{
    public:
        template<class State>
        const auto& get_state() const
        {
            return base_t::template get_state<State>();
        }

    private:
        template<class RegionListHolder>
        friend class subsm;

        template<class Derived, class TransitionTable2>
        friend class detail::region_impl;

        using base_t = detail::region_impl
        <
            region<TransitionTable>,
            TransitionTable
        >;

        using base_t::base_t;
};

} //namespace

#endif
