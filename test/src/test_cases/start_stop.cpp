//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#include <awesm.hpp>
#include "common/catch.hpp"
#include <string>

namespace
{
    struct context
    {
        std::string out;
    };

    namespace states
    {
        struct s0
        {
            void on_entry()
            {
                ctx.out += "s0::on_entry;";
            }

            void on_exit()
            {
                ctx.out += "s0::on_exit;";
            }

            context& ctx;
        };

        struct s1
        {
            void on_entry()
            {
                ctx.out += "s1::on_entry;";
            }

            void on_exit()
            {
                ctx.out += "s1::on_exit;";
            }

            context& ctx;
        };
    }

    namespace events
    {
        struct button_press{};
    }

    struct sm_transition_table
    {
        using type = awesm::transition_table
        <
            awesm::row<states::s0, awesm::none,          states::s1>,
            awesm::row<states::s1, events::button_press, states::s0>
        >;
    };

    using sm_t = awesm::simple_sm<sm_transition_table>;
}

TEST_CASE("start_stop")
{
    auto ctx = context{};
    auto sm = sm_t{ctx};

    REQUIRE(sm.is_active_state<awesm::null_state>());
    REQUIRE(ctx.out == "");

    sm.start();
    REQUIRE(sm.is_active_state<states::s1>());
    REQUIRE(ctx.out == "s0::on_entry;s0::on_exit;s1::on_entry;");

    ctx.out.clear();
    sm.stop();
    REQUIRE(sm.is_active_state<awesm::null_state>());
    REQUIRE(ctx.out == "s1::on_exit;");
}
