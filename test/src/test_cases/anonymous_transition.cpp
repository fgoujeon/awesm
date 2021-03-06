//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#include <awesm.hpp>
#include "common/catch.hpp"

namespace
{
    struct context{};

    struct events
    {
        struct go_on{};
    };

    struct states
    {
        struct s0{};
        struct s1{};
        struct s2{};
        struct s3{};
        struct s4{};
    };

    struct sm_transition_table
    {
        using type = awesm::transition_table
        <
            awesm::row<states::s0, events::go_on, states::s1>,
            awesm::row<states::s1, awesm::none,   states::s2>,
            awesm::row<states::s2, events::go_on, states::s3>,
            awesm::row<states::s3, awesm::none,   states::s4>,
            awesm::row<states::s4, awesm::none,   states::s0>
        >;
    };

    using sm_t = awesm::simple_sm<sm_transition_table>;
}

TEST_CASE("anonymous transition")
{
    auto ctx = context{};
    auto sm = sm_t{ctx};

    sm.start();

    sm.process_event(events::go_on{});
    REQUIRE(sm.is_active_state<states::s2>());

    sm.process_event(events::go_on{});
    REQUIRE(sm.is_active_state<states::s0>());
}
