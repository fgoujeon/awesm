//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_DETAIL_TLU_PUSH_BACK_HPP
#define AWESM_DETAIL_TLU_PUSH_BACK_HPP

namespace awesm::detail::tlu
{

/*
push_back adds a type to the back of a typelist.

In this example...:
    using typelist = tuple<char, short, int>;
    using typelist2 = push_back<typelist, long>;

... typelist2 is an alias of tuple<char, short, int, long>.
*/

namespace push_back_detail
{
    template<class TListT, class U>
    struct push_back;

    template<class U, template<class...> class TList, class... Ts>
    struct push_back<TList<Ts...>, U>
    {
        using type = TList<Ts..., U>;
    };
}

template<class TListT, class U>
using push_back = typename push_back_detail::push_back<TListT, U>::type;

} //namespace

#endif
