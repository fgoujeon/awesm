//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_SM_OPTIONS_ON_EXCEPTION_HPP
#define AWESM_SM_OPTIONS_ON_EXCEPTION_HPP

#include "../sm_ref.hpp"
#include "../detail/sm_object_holder.hpp"
#include <exception>

namespace awesm::sm_options
{

namespace detail::tags
{
    struct on_exception{};
}

namespace detail::defaults
{
    class on_exception
    {
        public:
            template<class Sm, class Context>
            on_exception(Sm& m, Context& /*ctx*/):
                sm_(m)
            {
            }

            void call(tags::on_exception /*tag*/, const std::exception_ptr& eptr, long /*priority*/)
            {
                sm_.process_event(eptr);
            }

        private:
            sm_ref<std::exception_ptr> sm_;
    };
}

template<class T>
class on_exception
{
    public:
        template<class Sm, class Context>
        on_exception(Sm& m, Context& ctx):
            impl_(m, ctx)
        {
        }

        template<class Event>
        void call(detail::tags::on_exception /*tag*/, const Event& event, int /*priority*/)
        {
            impl_.object.on_exception(event);
        }

    private:
        awesm::detail::sm_object_holder<T> impl_;
};

} //namespace

#endif