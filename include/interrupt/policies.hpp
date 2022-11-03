#pragma once

#include <boost/hana.hpp>

#include <type_traits>

namespace interrupt {
namespace hana = boost::hana;
using namespace hana::literals;

struct status_clear_policy {};

struct clear_status_first {
    using PolicyType = status_clear_policy;

    template <typename ClearStatusCallable, typename RunCallable>
    static void run(ClearStatusCallable const &clear_status,
                    RunCallable const &run) {
        clear_status();
        run();
    }
};

struct clear_status_last {
    using PolicyType = status_clear_policy;

    template <typename ClearStatusCallable, typename RunCallable>
    static void run(ClearStatusCallable const &clear_status,
                    RunCallable const &run) {
        run();
        clear_status();
    }
};

struct dont_clear_status {
    using PolicyType = status_clear_policy;

    template <typename ClearStatusCallable, typename RunCallable>
    static void run(ClearStatusCallable const &, RunCallable const &run) {
        run();
    }
};

struct required_resources_policy {};

template <typename... ResourcesT> struct required_resources {
    using PolicyType = required_resources_policy;

    constexpr static hana::tuple<ResourcesT...> resources{};
};

template <typename... PoliciesT> struct policies {
    constexpr static hana::tuple<PoliciesT...> values{};

    template <typename PolicyType, typename DefaultPolicy>
    constexpr static auto get() {
        auto const policy = hana::find_if(values, [](auto p) {
            using CurrentPolicyType = typename decltype(p)::PolicyType;
            return hana::bool_c<std::is_same_v<PolicyType, CurrentPolicyType>>;
        });

        if constexpr (policy == hana::nothing) {
            return DefaultPolicy{};
        } else {
            return *policy;
        }
    }

    template <typename PolicyType, typename DefaultPolicy>
    using type = decltype(get<PolicyType, DefaultPolicy>());
};
} // namespace interrupt
