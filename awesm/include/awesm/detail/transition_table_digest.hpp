//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_DETAIL_TRANSITION_TABLE_DIGEST_HPP
#define AWESM_DETAIL_TRANSITION_TABLE_DIGEST_HPP

#include "tlu.hpp"
#include "type_list.hpp"
#include "sm_object_holder_tuple.hpp"
#include "../null_state.hpp"
#include "../type_patterns.hpp"
#include "../none.hpp"
#include "../transition_table.hpp"
#include <type_traits>

namespace awesm::detail
{

/*
Creates a set of tuples containing all the action types, guard types and state
types of a given transition_table.

For example, the following digest type...:
    using transition_table = awesm::transition_table
    <
        awesm::row<state0, event0, state1>,
        awesm::row<state1, event1, state2, awesm::none, guard0>,
        awesm::row<state2, event2, state3, action0>,
        awesm::row<state3, event3, state0, action1,     guard1>
    >;
    using digest = awesm::detail::transition_table_digest<transition_table>;

... is equivalent to this type:
    struct digest
    {
        using state_tuple = awesm::detail::sm_object_holder_tuple<awesm::null_state, state0, state1, state2, state3>;
        using action_tuple = awesm::detail::sm_object_holder_tuple<action0, action1>;
        using guard_tuple = awesm::detail::sm_object_holder_tuple<guard0, guard1>;
        static constexpr auto has_source_state_patterns = false;
        static constexpr auto has_none_events = false;
    };
*/

namespace transition_table_digest_detail
{
    template<class TList>
    struct to_tuple_helper;

    template<template<class...> class TList, class... Ts>
    struct to_tuple_helper<TList<Ts...>>
    {
        using type = awesm::detail::sm_object_holder_tuple<Ts...>;
    };

    template<class TList>
    using to_tuple = typename to_tuple_helper<TList>::type;

    template<class TList, class U>
    using push_back_unique_if_not_pattern = tlu::push_back_if
    <
        TList,
        U,
        (
            !tlu::contains<TList, U> &&
            !std::is_base_of_v<type_pattern, U> &&
            !std::is_same_v<U, none>
        )
    >;

    struct initial_digest
    {
        using state_tuple = type_list<null_state>;
        using action_tuple = type_list<>;
        using guard_tuple = type_list<>;
        static constexpr auto has_source_state_patterns = false;
        static constexpr auto has_none_events = false;
    };

    template<class Digest, class Row>
    struct add_row_to_digest
    {
        using state_tuple = push_back_unique_if_not_pattern
        <
            push_back_unique_if_not_pattern
            <
                typename Digest::state_tuple,
                typename Row::source_state_type
            >,
            typename Row::target_state_type
        >;

        using action_tuple = push_back_unique_if_not_pattern
        <
            typename Digest::action_tuple,
            typename Row::action_type
        >;

        using guard_tuple = push_back_unique_if_not_pattern
        <
            typename Digest::guard_tuple,
            typename Row::guard_type
        >;

        static constexpr auto has_source_state_patterns =
            Digest::has_source_state_patterns ||
            std::is_base_of_v<type_pattern, typename Row::source_state_type>
        ;

        static constexpr auto has_none_events =
            Digest::has_none_events ||
            std::is_same_v<typename Row::event_type, none>
        ;
    };

    /*
    First step with type_list instead of awesm::detail::sm_object_holder_tuple,
    so that we don't instantiate intermediate tuples.
    */
    template<class TransitionTable>
    using digest_with_type_lists = tlu::left_fold
    <
        TransitionTable,
        add_row_to_digest,
        initial_digest
    >;
}

template<class TransitionTable>
class transition_table_digest
{
    private:
        using digest_t = transition_table_digest_detail::digest_with_type_lists
        <
            TransitionTable
        >;

    public:
        using state_tuple = transition_table_digest_detail::to_tuple
        <
            typename digest_t::state_tuple
        >;

        using action_tuple = transition_table_digest_detail::to_tuple
        <
            typename digest_t::action_tuple
        >;

        using guard_tuple = transition_table_digest_detail::to_tuple
        <
            typename digest_t::guard_tuple
        >;

        static constexpr auto has_source_state_patterns = digest_t::has_source_state_patterns;
        static constexpr auto has_none_events = digest_t::has_none_events;
};

} //namespace

#endif
