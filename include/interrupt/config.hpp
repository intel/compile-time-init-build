#pragma once

#include <interrupt/concepts.hpp>
#include <interrupt/dynamic_controller.hpp>
#include <interrupt/fwd.hpp>
#include <interrupt/impl.hpp>
#include <interrupt/policies.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/utility.hpp>

#include <boost/mp11/algorithm.hpp>

namespace interrupt {
namespace detail {
template <typename T>
concept control_config = requires {
    typename T::enable_field_t;
    typename T::status_field_t;
    { T::template enable<true, archetypes::hal>() } -> std::same_as<void>;
};

template <stdx::ct_string Type, irq_num_t Number, priority_t Priority>
struct mcu_control_config {
    template <bool Enable, typename Hal>
    constexpr static auto enable() -> void {
        Hal::template irq_init<Enable, Number, Priority>();
    }

    constexpr static auto irq_number = Number;
    using enable_field_t = no_field_t;
    using status_field_t = no_field_t;

    template <stdx::ct_string Name, typename Policies, typename... Flows>
    constexpr static auto config() {
        using namespace stdx::literals;
        return +stdx::ct_format<"interrupt::{}<\"{}\", {}_irq, {}, {}>">(
            stdx::ct<Type>(), stdx::ct<Name>(),
            stdx::ct<stdx::to_underlying(Number)>(), stdx::ct<Priority>(),
            detail::config_string_for<Policies, Flows...>());
    }
};

template <stdx::ct_string Type, typename EnableField, typename StatusField>
struct field_control_config {
    template <bool Enable, typename Hal>
    CONSTEVAL static auto enable() -> void {}

    using enable_field_t = EnableField;
    using status_field_t = StatusField;

    template <stdx::ct_string Name, typename Policies, typename... Flows>
    constexpr static auto config() {
        using namespace stdx::literals;
        return +stdx::ct_format<"interrupt::{}<\"{}\", {}>">(
            stdx::ct<Type>(), stdx::ct<Name>(),
            detail::config_string_for<EnableField, StatusField, Policies,
                                      Flows...>());
    }
};

static_assert(
    control_config<mcu_control_config<"", irq_num_t{}, priority_t{}>>);
static_assert(control_config<field_control_config<"", no_field_t, no_field_t>>);

template <typename Policies, typename... SubCfgs> struct policy_config {
    using status_policy_t =
        typename Policies::template type<status_clear_policy,
                                         clear_status_first>;
    using resources_t =
        typename Policies::template type<required_resources_policy,
                                         required_resources<>>::resources;

    using resource_children_t = stdx::tuple<SubCfgs...>;

    using all_resources_t = boost::mp11::mp_unique<
        boost::mp11::mp_append<resources_t, typename SubCfgs::resources_t...>>;
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

template <typename... Flows> struct flow_config {
  private:
    template <typename Flow, typename... Nexi>
    constexpr static auto one_active() {
        return (... or Nexi::template service<Flow>.active);
    }

    template <typename Flow, typename... Nexi>
    constexpr static auto one_isr() -> void {
        (Nexi::template service<Flow>(), ...);
    }

  public:
    using flows_t = stdx::type_list<Flows...>;
    using all_flows_t = flows_t;

    template <typename... Nexi>
    constexpr static bool active = (... or one_active<Flows, Nexi...>());

    template <typename Nexus>
    constexpr static bool has_flows_for = (... and nexus_for<Nexus, Flows>);

    template <typename... Nexi> constexpr static auto isr() -> void {
        (one_isr<Flows, Nexi...>(), ...);
    }
};

template <typename... Flows>
struct container_flow_config : flow_config<Flows...> {
    using flows_t = stdx::type_list<>;
};
} // namespace detail

template <base_irq_config... Cfgs>
struct root : detail::parent_config<Cfgs...> {
    template <typename... Ts>
    using dynamic_controller_t = dynamic_controller<Ts...>;
};

namespace detail {
template <stdx::ct_string Name, typename ControlCfg, typename Policies,
          typename... Flows>
struct element_irq : policy_config<Policies>,
                     detail::parent_config<>,
                     ControlCfg,
                     detail::flow_config<Flows...> {
    using name_t = stdx::cts_t<Name>;

    template <typename... Nexi>
    using built_t = element_irq_impl<element_irq, Nexi...>;

    constexpr static auto config() {
        return ControlCfg::template config<Name, Policies, Flows...>();
    }
};
} // namespace detail

template <stdx::ct_string Name, irq_num_t Number, priority_t Priority,
          typename Policies, typename... Flows>
using irq =
    detail::element_irq<Name,
                        detail::mcu_control_config<"irq", Number, Priority>,
                        Policies, Flows...>;

template <stdx::ct_string Name, typename EnableField, typename StatusField,
          typename Policies, typename... Flows>
using sub_irq = detail::element_irq<
    Name, detail::field_control_config<"sub_irq", EnableField, StatusField>,
    Policies, Flows...>;

namespace detail {
template <stdx::ct_string Name, typename ControlCfg, typename Policies,
          sub_irq_config... Cfgs>
struct container_irq
    : policy_config<Policies, Cfgs...>,
      detail::parent_config<Cfgs...>,
      ControlCfg,
      boost::mp11::mp_apply<detail::container_flow_config,
                            boost::mp11::mp_unique<boost::mp11::mp_append<
                                typename Cfgs::all_flows_t...>>> {
    using name_t = stdx::cts_t<Name>;

    template <typename... Subs>
    using sub_built_t = container_irq_impl<container_irq, Subs...>;

    template <typename... Nexi>
    using built_t =
        typename detail::parent_config<Cfgs...>::template build<sub_built_t,
                                                                Nexi...>;

    constexpr static auto config() {
        return ControlCfg::template config<Name, Policies, Cfgs...>();
    }
};
} // namespace detail

template <stdx::ct_string Name, irq_num_t Number, priority_t Priority,
          typename Policies, sub_irq_config... Cfgs>
using shared_irq = detail::container_irq<
    Name, detail::mcu_control_config<"shared_irq", Number, Priority>, Policies,
    Cfgs...>;

template <stdx::ct_string Name, typename EnableField, typename StatusField,
          typename Policies, sub_irq_config... Cfgs>
using shared_sub_irq = detail::container_irq<
    Name,
    detail::field_control_config<"shared_sub_irq", EnableField, StatusField>,
    Policies, Cfgs...>;
} // namespace interrupt
