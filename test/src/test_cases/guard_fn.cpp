//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/fgfsm

#include <fgfsm.hpp>
#include <catch2/catch.hpp>
#include <string>

namespace
{
    struct context
    {
    };

    struct some_event
    {
        int value = 0;
    };

    struct is_positive_int_1
    {
        bool check(const some_event& event)
        {
            return event.value > 0;
        }

        context& ctx;
    };

    bool is_positive_int_2_impl(context& ctx, const some_event& event)
    {
        return event.value > 0;
    }
    using is_positive_int_2 = fgfsm::guard_fn<is_positive_int_2_impl>;
}

TEST_CASE("guard_fn")
{
    auto ctx = context{};
    auto guard_1 = is_positive_int_1{ctx};
    auto guard_2 = is_positive_int_2{ctx};

    {
        const auto event = some_event{1};
        REQUIRE(guard_1.check(event));
        REQUIRE(guard_2.check(event));
    }

    {
        const auto event = some_event{-1};
        REQUIRE(!guard_1.check(event));
        REQUIRE(!guard_2.check(event));
    }
}
