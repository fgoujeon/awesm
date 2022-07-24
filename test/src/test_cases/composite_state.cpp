//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#include <awesm.hpp>
#include "common/catch.hpp"

namespace
{
    struct sm_transition_table;
    using sm_t = awesm::simple_sm<sm_transition_table>;

    enum class led_color
    {
        off,
        red,
        green,
        blue
    };

    struct context
    {
        led_color current_led_color = led_color::off;
    };

    namespace events
    {
        struct power_button_press{};
        struct color_button_press{};
    }

    namespace states
    {
        struct off{};

        struct emitting_red
        {
            void on_entry()
            {
                ctx.current_led_color = led_color::red;
            }

            void on_exit()
            {
            }

            sm_t& sm;
            context& ctx;
        };

        struct emitting_green
        {
            void on_entry()
            {
                ctx.current_led_color = led_color::green;
            }

            void on_exit()
            {
            }

            sm_t& sm;
            context& ctx;
        };

        struct emitting_blue
        {
            void on_entry()
            {
                ctx.current_led_color = led_color::blue;
            }

            void on_exit()
            {
            }

            sm_t& sm;
            context& ctx;
        };

        using on_transition_table = awesm::transition_table
        <
            awesm::row<states::emitting_red,   events::color_button_press, states::emitting_green>,
            awesm::row<states::emitting_green, events::color_button_press, states::emitting_blue>,
            awesm::row<states::emitting_blue,  events::color_button_press, states::emitting_red>
        >;

        struct on_def
        {
            template<class Event>
            void on_entry(const Event& /*event*/)
            {
            }

            template<class Event>
            void on_event(const Event& /*event*/)
            {
            }

            template<class Event>
            void on_exit(const Event& /*event*/)
            {
                ctx.current_led_color = led_color::off;
            }

            context& ctx;
        };

        using on = awesm::simple_composite_state<on_def, on_transition_table>;
    }

    struct sm_transition_table
    {
        using type = awesm::transition_table
        <
            awesm::row<states::off, events::power_button_press, states::on>,
            awesm::row<states::on,  events::power_button_press, states::off>
        >;
    };
}

TEST_CASE("composite_state")
{
    auto ctx = context{};
    auto sm = sm_t{ctx};

    sm.start();

    REQUIRE(sm.is_active_state<states::off>());
    REQUIRE(sm.get_state<states::on>().is_active_state<awesm::null_state>());
    REQUIRE(ctx.current_led_color == led_color::off);

    sm.process_event(events::power_button_press{});
    REQUIRE(sm.is_active_state<states::on>());
    REQUIRE(sm.get_state<states::on>().is_active_state<states::emitting_red>());
    REQUIRE(ctx.current_led_color == led_color::red);

    sm.process_event(events::color_button_press{});
    REQUIRE(sm.is_active_state<states::on>());
    REQUIRE(ctx.current_led_color == led_color::green);

    sm.process_event(events::color_button_press{});
    REQUIRE(sm.is_active_state<states::on>());
    REQUIRE(ctx.current_led_color == led_color::blue);

    sm.process_event(events::power_button_press{});
    REQUIRE(sm.is_active_state<states::off>());
    REQUIRE(ctx.current_led_color == led_color::off);

    sm.process_event(events::power_button_press{});
    REQUIRE(sm.is_active_state<states::on>());
    REQUIRE(ctx.current_led_color == led_color::red);
}
