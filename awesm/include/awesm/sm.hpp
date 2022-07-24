//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_SM_HPP
#define AWESM_SM_HPP

#include "detail/region_tuple.hpp"
#include "detail/sm_configuration.hpp"
#include "detail/event_processing_type.hpp"
#include "detail/alternative_lazy.hpp"
#include "detail/any_container.hpp"
#include <queue>
#include <type_traits>

namespace awesm
{

namespace detail
{
    class false_at_destruction_setter
    {
        public:
            false_at_destruction_setter(bool& boolean):
                b_(boolean)
            {
            }

            false_at_destruction_setter(const false_at_destruction_setter&) = delete;
            false_at_destruction_setter(false_at_destruction_setter&&) = delete;
            false_at_destruction_setter& operator=(const false_at_destruction_setter&) = delete;
            false_at_destruction_setter& operator=(false_at_destruction_setter&&) = delete;

            ~false_at_destruction_setter()
            {
                b_ = false;
            }

        private:
            bool& b_;
    };
}

template<class RegionListHolder, class... Options>
class sm
{
    public:
        template<class Context>
        explicit sm(Context& context):
            conf_(*this, context),
            region_tuple_{*this, context}
        {
        }

        sm(const sm&) = delete;
        sm(sm&&) = delete;
        sm& operator=(const sm&) = delete;
        sm& operator=(sm&&) = delete;
        ~sm() = default;

        template<int RegionIndex = 0>
        const auto& get_region() const
        {
            return region_tuple_.template get_region<RegionIndex>();
        }

        template<class State, int RegionIndex = 0>
        const auto& get_state() const
        {
            return region_tuple_.template get_state<State, RegionIndex>();
        }

        template<class State, int RegionIndex = 0>
        [[nodiscard]] bool is_active_state() const
        {
            return region_tuple_.template is_active_state<State, RegionIndex>();
        }

        template<class Event = none>
        void start(const Event& event = {})
        {
            process_event_2<detail::event_processing_type::start>(event);
        }

        template<class Event = none>
        void stop(const Event& event = {})
        {
            process_event_2<detail::event_processing_type::stop>(event);
        }

        template<class Event>
        void process_event(const Event& event)
        {
            process_event_2<detail::event_processing_type::event>(event);
        }

    private:
        using configuration_t = detail::sm_configuration
        <
            sm_options::detail::defaults::after_state_transition,
            sm_options::detail::defaults::before_entry,
            sm_options::detail::defaults::before_state_transition,
            sm_options::detail::defaults::in_state_internal_transitions,
            sm_options::detail::defaults::on_event,
            sm_options::detail::defaults::on_exception,
            sm_options::detail::defaults::run_to_completion,
            Options...
        >;

        class event_processing
        {
            public:
                template<class Event, detail::event_processing_type ProcessingType>
                event_processing
                (
                    sm& machine,
                    const Event& event,
                    std::integral_constant<detail::event_processing_type, ProcessingType> /*unused*/
                ):
                    sm_(machine),
                    event_(event),
                    pprocess_event_
                    (
                        [](sm& mach, const event_storage_t& evt)
                        {
                            mach.process_event_once<ProcessingType>(evt.get<Event>());
                        }
                    )
                {
                }

                event_processing(const event_processing&) = delete;
                event_processing(event_processing&&) = delete;
                ~event_processing() = default;
                event_processing& operator=(const event_processing&) = delete;
                event_processing& operator=(event_processing&&) = delete;

                void operator()()
                {
                    (*pprocess_event_)(sm_, event_);
                }

            private:
                static constexpr auto small_event_size = 16;
                using event_storage_t = detail::any_container<small_event_size>;

                sm& sm_;
                event_storage_t event_;
                void(*pprocess_event_)(sm&, const event_storage_t&) = nullptr;
        };

        struct queue_holder
        {
            template<class = void>
            using type = std::queue<event_processing>;
        };
        struct empty_holder
        {
            template<class = void>
            struct type{};
        };
        using queued_event_processing_storage_t = detail::alternative_lazy
        <
            configuration_t::has_run_to_completion(),
            queue_holder,
            empty_holder
        >;

        template<detail::event_processing_type ProcessingType, class Event>
        void process_event_2(const Event& event)
        {
            if constexpr(configuration_t::has_run_to_completion())
            {
                //Queue event processing in case of recursive call
                if(processing_event_)
                {
                    safe_call //copy constructor might throw
                    (
                        [&]
                        {
                            queued_event_processings_.emplace
                            (
                                *this,
                                event,
                                std::integral_constant
                                <
                                    detail::event_processing_type,
                                    ProcessingType
                                >{}
                            );
                        }
                    );
                    return;
                }

                processing_event_ = true;
                auto setter = detail::false_at_destruction_setter{processing_event_};

                process_event_once<ProcessingType>(event);

                //Process deferred event processings
                while(!queued_event_processings_.empty())
                {
                    queued_event_processings_.front()();
                    queued_event_processings_.pop();
                }
            }
            else
            {
                process_event_once<ProcessingType>(event);
            }
        }

        //Used to call client code
        template<class F>
        void safe_call(F&& callback)
        {
            try
            {
                callback();
            }
            catch(...)
            {
                conf_.on_exception(std::current_exception());
            }
        }

        template<detail::event_processing_type ProcessingType, class Event>
        void process_event_once(const Event& event)
        {
            if constexpr(ProcessingType == detail::event_processing_type::event)
            {
                safe_call
                (
                    [&]
                    {
                        conf_.on_event(&event);
                    }
                );
            }

            if constexpr(ProcessingType == detail::event_processing_type::start)
            {
                region_tuple_.start(conf_, event);
            }
            else if constexpr(ProcessingType == detail::event_processing_type::stop)
            {
                region_tuple_.stop(conf_, event);
            }
            else
            {
                region_tuple_.process_event(conf_, event);
            }
        }

        configuration_t conf_;
        detail::region_tuple<typename RegionListHolder::type> region_tuple_;

        bool processing_event_ = false;
        queued_event_processing_storage_t queued_event_processings_;
};

} //namespace

#endif
