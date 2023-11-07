#pragma once

#include <stdx/concepts.hpp>
#include <stdx/tuple.hpp>

#include <utility>

namespace interrupt {
template <typename T>
concept status_policy = requires(void (*f)()) {
    typename T::policy_type;
    { T ::run(f, f) } -> stdx::same_as<void>;
};

struct status_clear_policy {};

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

struct required_resources_policy {};

template <typename... ResourcesT> struct required_resources {
    using policy_type = required_resources_policy;

    constexpr static stdx::tuple<ResourcesT...> resources{};
};

template <typename... Policies> class policies {
    template <typename Key, typename Value> struct type_pair {};
    template <typename... Ts> struct type_map : Ts... {};

    template <typename K, typename Default>
    constexpr static auto lookup(...) -> Default;
    template <typename K, typename Default, typename V>
    constexpr static auto lookup(type_pair<K, V>) -> V;

  public:
    template <typename PolicyType, typename Default>
    constexpr static auto get() {
        using M =
            type_map<type_pair<typename Policies::policy_type, Policies>...>;
        return decltype(lookup<PolicyType, Default>(std::declval<M>())){};
    }

    template <typename PolicyType, typename Default>
    using type = decltype(get<PolicyType, Default>());
};
} // namespace interrupt
