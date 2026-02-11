#pragma once

#include <interrupt/concepts.hpp>
#include <interrupt/dynamic_controller.hpp>
#include <interrupt/fwd.hpp>
#include <interrupt/impl.hpp>
#include <interrupt/policies.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/utility.hpp>

namespace interrupt {
namespace detail {
template <stdx::ct_string Name, typename Policies> struct base_config {
    using status_policy_t =
        typename Policies::template type<status_clear_policy,
                                         clear_status_first>;
    using resources_t =
        typename Policies::template type<required_resources_policy,
                                         required_resources<>>::resources;
    using name_t = stdx::cts_t<Name>;
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
    template <bool Enable, typename Hal>
    constexpr static auto enable() -> void {
        Hal::template irq_init<Enable, Number, Priority>();
    }
    constexpr static auto irq_number = Number;
};

template <typename EnableField, typename StatusField> struct sub_config {
    template <bool Enable, typename Hal>
    constexpr static auto enable() -> void {}
    using enable_field_t = EnableField;
    using status_field_t = StatusField;
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
    using flows_t = stdx::type_list<Flows...>;

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
    template <typename... Ts>
    using dynamic_controller_t = dynamic_controller<Ts...>;
};

template <stdx::ct_string Name, irq_num_t Number, priority_t Priority,
          typename Policies, typename... Flows>
struct irq : detail::base_config<Name, Policies>,
             detail::parent_config<>,
             detail::super_config<Number, Priority>,
             detail::flow_config<Flows...> {
    template <typename... Nexi> using built_t = irq_impl<irq, Nexi...>;

    constexpr static auto config() {
        using namespace stdx::literals;
        return +stdx::ct_format<"interrupt::irq<\"{}\", {}_irq, {}, {}>">(
            stdx::ct<Name>(), stdx::ct<stdx::to_underlying(Number)>(),
            stdx::ct<Priority>(),
            detail::config_string_for<Policies, Flows...>());
    }
};

template <stdx::ct_string Name, typename EnableField, typename StatusField,
          typename Policies, typename... Flows>
struct sub_irq : detail::base_config<Name, Policies>,
                 detail::parent_config<>,
                 detail::sub_config<EnableField, StatusField>,
                 detail::flow_config<Flows...> {
    template <typename... Nexi> using built_t = sub_irq_impl<sub_irq, Nexi...>;

    constexpr static auto config() {
        using namespace stdx::literals;
        return +stdx::ct_format<"interrupt::sub_irq<\"{}\", {}>">(
            stdx::ct<Name>(),
            detail::config_string_for<EnableField, StatusField, Policies,
                                      Flows...>());
    }
};

template <stdx::ct_string Name, typename EnableField, typename Policies>
struct id_irq : detail::base_config<Name, Policies>,
                detail::parent_config<>,
                detail::sub_config<EnableField, no_field_t>,
                detail::flow_config<> {
    template <typename...> using built_t = id_irq_impl<id_irq>;
    template <typename> constexpr static bool triggers_flow = false;

    constexpr static auto config() {
        using namespace stdx::literals;
        return +stdx::ct_format<"interrupt::id_irq<\"{}\", {}>">(
            stdx::ct<Name>(),
            detail::config_string_for<EnableField, Policies>());
    }
};

template <stdx::ct_string Name, irq_num_t Number, priority_t Priority,
          typename Policies, sub_irq_config... Cfgs>
struct shared_irq : detail::base_config<Name, Policies>,
                    detail::parent_config<Cfgs...>,
                    detail::super_config<Number, Priority>,
                    detail::flow_config<> {
    template <typename... Subs>
    using sub_built_t = shared_irq_impl<shared_irq, Subs...>;

    template <typename... Nexi>
    using built_t =
        typename detail::parent_config<Cfgs...>::template build<sub_built_t,
                                                                Nexi...>;

    template <typename Flow>
    constexpr static bool triggers_flow =
        (... or Cfgs::template triggers_flow<Flow>);

    constexpr static auto config() {
        using namespace stdx::literals;
        return +stdx::ct_format<
            "interrupt::shared_irq<\"{}\", {}_irq, {}, {}>">(
            stdx::ct<Name>(), stdx::ct<stdx::to_underlying(Number)>(),
            stdx::ct<Priority>(),
            detail::config_string_for<Policies, Cfgs...>());
    }
};

template <stdx::ct_string Name, typename EnableField, typename StatusField,
          typename Policies, sub_irq_config... Cfgs>
struct shared_sub_irq : detail::base_config<Name, Policies>,
                        detail::parent_config<Cfgs...>,
                        detail::sub_config<EnableField, StatusField>,
                        detail::flow_config<> {
    template <typename... Subs>
    using sub_built_t = shared_sub_irq_impl<shared_sub_irq, Subs...>;

    template <typename... Nexi>
    using built_t =
        typename detail::parent_config<Cfgs...>::template build<sub_built_t,
                                                                Nexi...>;

    template <typename Flow>
    constexpr static bool triggers_flow =
        (... or Cfgs::template triggers_flow<Flow>);

    constexpr static auto config() {
        using namespace stdx::literals;
        return +stdx::ct_format<"interrupt::shared_sub_irq<\"{}\", {}>">(
            stdx::ct<Name>(),
            detail::config_string_for<EnableField, StatusField, Policies,
                                      Cfgs...>());
    }
};
} // namespace interrupt
