//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/fgfsm

#ifndef FGFSM_SIMPLE_STATE_HPP
#define FGFSM_SIMPLE_STATE_HPP

#define FGFSM_SIMPLE_STATE(NAME) \
    struct NAME \
    { \
        template<class Context> \
        NAME(Context&) \
        { \
        } \
 \
        void on_entry(const fgfsm::any_cref&) \
        { \
        } \
 \
        void on_event(const fgfsm::any_cref&) \
        { \
        } \
 \
        void on_exit(const fgfsm::any_cref&) \
        { \
        } \
    };

#endif