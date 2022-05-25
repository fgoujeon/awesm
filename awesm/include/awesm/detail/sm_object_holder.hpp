//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_DETAIL_SM_OBJECT_HOLDER_HPP
#define AWESM_DETAIL_SM_OBJECT_HOLDER_HPP

#include <type_traits>

namespace awesm::detail
{

namespace sm_object_holder_detail
{
    template<class T, class... Args>
    constexpr auto is_brace_constructible_impl(int) -> decltype(T{std::declval<Args>()...}, bool())
    {
        return true;
    }

    template<class T, class... Args>
    constexpr bool is_brace_constructible_impl(long)
    {
        return false;
    }

    template<class T, class... Args>
    constexpr bool is_brace_constructible =
        is_brace_constructible_impl<T, Args...>(0)
    ;
}

/*
SM objects are states, actions and guards.
All SM objects can be constructed with one of these statements:
    auto obj = object_type{context};
    auto obj = object_type{};
Since we don't want to require SM objects to be copyable or movable, we can't
use a function such as make_sm_object<T>(context). We need this wrapper to
construct the objects.
*/
template<class T>
struct sm_object_holder
{
    template<class Context, class Sm, class U = T>
    sm_object_holder
    (
        Context& ctx,
        Sm& machine,
        std::enable_if_t
        <
            sm_object_holder_detail::is_brace_constructible<U, Context&, Sm&>
        >* = nullptr
    ):
        object{ctx, machine}
    {
    }

    template<class Context, class Sm, class U = T>
    sm_object_holder
    (
        Context& ctx,
        Sm&,
        std::enable_if_t
        <
            sm_object_holder_detail::is_brace_constructible<U, Context&>
        >* = nullptr
    ):
        object{ctx}
    {
    }

    template<class Context, class Sm, class U = T>
    sm_object_holder
    (
        Context&,
        Sm&,
        std::enable_if_t<std::is_default_constructible_v<U>>* = nullptr
    )
    {
    }

    T object;
};

} //namespace

#endif