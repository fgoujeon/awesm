//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#include <awesm.hpp>
#include <catch2/catch.hpp>

namespace
{
    struct sm_configuration;
    using sm_t = awesm::simple_sm<sm_configuration>;

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

    namespace actions
    {
        template<led_color Color>
        struct set_led_color
        {
            void execute()
            {
                ctx.current_led_color = Color;
            }

            context& ctx;
        };
    };

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

        class on
        {
            public:
                on(sm_t& sm, context& ctx):
                    subsm_(sm, ctx),
                    ctx_(ctx)
                {
                }

                void on_entry()
                {
                    subsm_.reset();
                    subsm_.start();
                }

                template<class Event>
                void on_event(const Event& event)
                {
                    subsm_.process_event(event);
                }

                void on_exit()
                {
                    subsm_.stop();
                    ctx_.current_led_color = led_color::off;
                }

            private:
                struct region_conf: awesm::region_configuration
                {
                    using transition_table = awesm::transition_table
                    <
                        awesm::row<states::emitting_red,   events::color_button_press, states::emitting_green>,
                        awesm::row<states::emitting_green, events::color_button_press, states::emitting_blue>,
                        awesm::row<states::emitting_blue,  events::color_button_press, states::emitting_red>
                    >;
                };

                struct subsm_configuration: awesm::subsm_configuration
                {
                    using region_configurations = awesm::region_configuration_list
                    <
                        region_conf
                    >;
                };

                using subsm_t = awesm::subsm<subsm_configuration>;

                subsm_t subsm_;
                context& ctx_;
        };
    }

    struct sm_configuration: awesm::simple_sm_configuration
    {
        using transition_table = awesm::transition_table
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

    REQUIRE(sm.is_active_state<states::off>());
    REQUIRE(ctx.current_led_color == led_color::off);

    sm.process_event(events::power_button_press{});
    REQUIRE(sm.is_active_state<states::on>());
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