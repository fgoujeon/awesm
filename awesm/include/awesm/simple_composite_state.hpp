//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_SIMPLE_COMPOSITE_STATE_HPP
#define AWESM_SIMPLE_COMPOSITE_STATE_HPP

#include "composite_state.hpp"
#include "region_list.hpp"

namespace awesm
{

template<class Definition, class TransitionTable>
using simple_composite_state = composite_state
<
    Definition,
    region_list<region<TransitionTable>>
>;

} //namespace

#endif
