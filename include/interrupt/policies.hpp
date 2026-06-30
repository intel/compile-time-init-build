#pragma once

#include <stdx/concepts.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/utility.hpp>

#include <boost/mp11/list.hpp>

namespace interrupt {
template <typename T>
concept policy = requires { typename T::policy_type; };

template <typename T>
concept status_policy = policy<T> and requires(void (*f)()) {
    { T::run(f, f) } -> stdx::same_as<void>;
};

struct status_clear_policy;

struct clear_status_first {
    using policy_type = status_clear_policy;

    static void run(stdx::invocable auto const &clear_status,
                    stdx::invocable auto const &run) {
        clear_status();
        run();
    }

    static_assert(status_policy<clear_status_first>);
};

struct clear_status_last {
    using policy_type = status_clear_policy;

    static void run(stdx::invocable auto const &clear_status,
                    stdx::invocable auto const &run) {
        run();
        clear_status();
    }

    static_assert(status_policy<clear_status_last>);
};

struct dont_clear_status {
    using policy_type = status_clear_policy;

    static void run(stdx::invocable auto const &,
                    stdx::invocable auto const &run) {
        run();
    }

    static_assert(status_policy<dont_clear_status>);
};

struct required_resources_policy;
template <typename... Resources> struct resource_list {};

template <typename T>
concept resources_policy =
    policy<T> and
    stdx::is_specialization_of_v<typename T::resources, resource_list>;

template <typename... Resources> struct required_resources {
    using policy_type = required_resources_policy;
    using resources = resource_list<Resources...>;
};

template <typename... Policies> struct policies {
    template <typename PolicyType, typename Default>
    constexpr static auto get() {
        using M = stdx::type_map<
            stdx::tt_pair<typename Policies::policy_type, Policies>...>;
        return stdx::type_lookup_t<M, PolicyType, Default>{};
    }

    template <typename PolicyType, typename Default>
    using type = decltype(get<PolicyType, Default>());
};

struct dynamic_enable_policy {
    template <typename Irq, typename Names, typename Rsrcs, typename Flows>
    [[nodiscard]] constexpr static auto is_on(Names const &named_enables,
                                              Rsrcs const &resource_enables,
                                              Flows const &flow_enables)
        -> bool {
        // an IRQ is OFF if it is disabled by name
        if (not named_enables[stdx::type_identity_v<typename Irq::name_t>]) {
            return false;
        }

        // or if it has resources and any are OFF
        constexpr auto has_resources =
            not boost::mp11::mp_empty<typename Irq::resources_t>::value;
        if constexpr (has_resources) {
            constexpr auto resources_bs = Rsrcs{typename Irq::resources_t{}};
            if ((resource_enables & resources_bs) != resources_bs) {
                return false;
            }
        }

        // otherwise, an IRQ is ON if:
        // ANY of its flows are ON
        constexpr auto flows_bs = Flows{typename Irq::flows_t{}};
        auto const on_by_flows = (flow_enables & flows_bs).any();

        // or ANY of its children are ON (by the same reasoning)
        auto const on_by_children = stdx::any_of(
            [&]<typename Child>(Child) {
                return is_on<Child>(named_enables, resource_enables,
                                    flow_enables);
            },
            Irq::children);

        return on_by_flows or on_by_children;
    }
};

namespace dynamic_init {
struct all_resources_policy {
    template <typename Resources>
    using active_resources_t =
        boost::mp11::mp_apply<stdx::type_list, Resources>;
};
struct all_flows_policy {
    template <typename Flows>
    using active_flows_t = boost::mp11::mp_apply<stdx::type_list, Flows>;
};
struct all_irqs_policy {
    template <typename Irqs>
    using active_irqs_t = boost::mp11::mp_apply<stdx::type_list, Irqs>;
};

template <typename... Resources> struct resources_policy {
    template <typename>
    using active_resources_t = stdx::type_list<Resources...>;
};
template <typename... Flows> struct flows_policy {
    template <typename> using active_flows_t = stdx::type_list<Flows...>;
};
template <typename... Irqs> struct irqs_policy {
    template <typename> using active_irqs_t = stdx::type_list<Irqs...>;
};

using no_resources_policy = resources_policy<>;
using no_flows_policy = flows_policy<>;
using no_irqs_policy = irqs_policy<>;

struct default_policy : all_resources_policy,
                        all_flows_policy,
                        all_irqs_policy {
    constexpr static auto dynamic_enable_top_level = false;
};
} // namespace dynamic_init
} // namespace interrupt
