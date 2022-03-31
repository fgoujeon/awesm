//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/fgfsm

#include <fgfsm.hpp>
#include <catch2/catch.hpp>

namespace
{
    struct context
    {
        bool has_power = false;
    };

    namespace states
    {
        FGFSM_SIMPLE_STATE(on)
        FGFSM_SIMPLE_STATE(off)
    }

    namespace events
    {
        struct button_press
        {
            bool hard = false;
        };
    }

    namespace guards
    {
        struct has_power
        {
            bool check(const fgfsm::any_cref&)
            {
                return ctx.has_power;
            }

            context& ctx;
        };

        bool is_pressing_hard_impl(context&, const events::button_press& event)
        {
            return event.hard;
        }
        using is_pressing_hard = fgfsm::guard_fn<is_pressing_hard_impl>;
    }

    using transition_table = fgfsm::transition_table
    <
        fgfsm::row<states::off, events::button_press, states::on,  fgfsm::none, guards::has_power>,
        fgfsm::row<states::on,  events::button_press, states::off, fgfsm::none, guards::is_pressing_hard>
    >;

    using fsm = fgfsm::fsm<transition_table>;
}

TEST_CASE("guard")
{
    auto ctx = context{};
    auto sm = fsm{ctx};

    sm.process_event(events::button_press{});
    REQUIRE(sm.is_active_state<states::off>());

    ctx.has_power = true;
    sm.process_event(events::button_press{true});
    REQUIRE(sm.is_active_state<states::on>());
}
