#pragma once

#include <interrupt/concepts.hpp>
#include <interrupt/dynamic_controller.hpp>
#include <interrupt/fwd.hpp>
#include <interrupt/hal.hpp>
#include <interrupt/impl.hpp>
#include <interrupt/policies.hpp>

#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

namespace interrupt {
namespace detail {
template <typename Policies> struct policy_config {
    using status_policy_t =
        typename Policies::template type<status_clear_policy,
                                         clear_status_first>;
    using resources_t =
        typename Policies::template type<required_resources_policy,
                                         required_resources<>>::resources;
};

template <base_irq_config... Cfgs> struct parent_config {
    constexpr static auto children = stdx::tuple<Cfgs...>{};
    constexpr static auto descendants =
        stdx::tuple_cat(children, children.apply([](auto... child) {
            return stdx::tuple_cat(child.descendants...);
        }));

    template <template <typename...> typename C, typename... Ts>
    using build = C<typename Cfgs::template built_t<Ts...>...>;
};

template <irq_num_t Number, priority_t Priority> struct super_config {
    template <bool Enable> constexpr static auto enable() -> void {
        hal::irq_init<Enable, Number, Priority>();
    }
    constexpr static auto irq_number = Number;
};

template <typename EnableField, typename StatusField> struct sub_config {
    template <bool Enable> constexpr static auto enable() -> void {}
    constexpr static auto enable_field = EnableField{};
    constexpr static auto status_field = StatusField{};
};

template <typename... Flows> class flow_config {
    template <typename Flow, typename... Nexi>
    constexpr static auto one_active() {
        return (... or Nexi::template service<Flow>.active);
    }

    template <typename Flow, typename... Nexi>
    constexpr static auto one_isr() -> void {
        (Nexi::template service<Flow>(), ...);
    }

  public:
    template <typename... Nexi>
    constexpr static bool active = (... or one_active<Flows, Nexi...>());

    template <typename Nexus>
    constexpr static bool has_flows_for = (... and nexus_for<Nexus, Flows>);

    template <typename Flow>
    constexpr static bool triggers_flow = (... or std::same_as<Flow, Flows>);

    template <typename... Nexi> constexpr static auto isr() -> void {
        (one_isr<Flows, Nexi...>(), ...);
    }
};
} // namespace detail

template <base_irq_config... Cfgs>
struct root : detail::parent_config<Cfgs...> {
    template <typename T> using dynamic_controller_t = dynamic_controller<T>;
};

template <irq_num_t Number, priority_t Priority, typename Policies,
          typename... Flows>
struct irq : detail::policy_config<Policies>,
             detail::parent_config<>,
             detail::super_config<Number, Priority>,
             detail::flow_config<Flows...> {
    template <typename... Nexi> using built_t = irq_impl<irq, Nexi...>;
};

template <typename EnableField, typename StatusField, typename Policies,
          typename... Flows>
struct sub_irq : detail::policy_config<Policies>,
                 detail::parent_config<>,
                 detail::sub_config<EnableField, StatusField>,
                 detail::flow_config<Flows...> {
    template <typename... Nexi> using built_t = sub_irq_impl<sub_irq, Nexi...>;
};

template <irq_num_t Number, priority_t Priority, typename Policies,
          sub_irq_config... Cfgs>
struct shared_irq : detail::policy_config<Policies>,
                    detail::parent_config<Cfgs...>,
                    detail::super_config<Number, Priority> {
    template <typename... Subs>
    using sub_built_t = shared_irq_impl<shared_irq, Subs...>;

    template <typename... Nexi>
    using built_t =
        typename detail::parent_config<Cfgs...>::template build<sub_built_t,
                                                                Nexi...>;

    template <typename Flow>
    constexpr static bool triggers_flow =
        (... or Cfgs::template triggers_flow<Flow>);
};

template <typename EnableField, typename StatusField, typename Policies,
          sub_irq_config... Cfgs>
struct shared_sub_irq : detail::policy_config<Policies>,
                        detail::parent_config<Cfgs...>,
                        detail::sub_config<EnableField, StatusField> {
    template <typename... Subs>
    using sub_built_t = shared_sub_irq_impl<shared_sub_irq, Subs...>;

    template <typename... Nexi>
    using built_t =
        typename detail::parent_config<Cfgs...>::template build<sub_built_t,
                                                                Nexi...>;

    template <typename Flow>
    constexpr static bool triggers_flow =
        (... or Cfgs::template triggers_flow<Flow>);
};
} // namespace interrupt
