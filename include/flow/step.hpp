#pragma once

#include <cib/detail/runtime_conditional.hpp>
#include <cib/func_decl.hpp>
#include <flow/common.hpp>
#include <flow/log.hpp>
#include <flow/subgraph_identity.hpp>
#include <sc/string_constant.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/type_traits.hpp>

#include <type_traits>

namespace flow {
struct rt_node {
    FunctionPtr run;
    FunctionPtr log_name;

  private:
    friend constexpr auto operator==(rt_node const &,
                                     rt_node const &) -> bool = default;
};

namespace detail {
template <typename Cond, typename F> constexpr auto run_func() -> void {
    if (Cond{}) {
        F{}();
    }
}

template <typename ct_node, typename Cond, stdx::ct_string Type,
          stdx::ct_string Name>
constexpr auto log_func() -> void {
    if (Cond{}) {
        using log_spec_t = decltype(get_log_spec<ct_node>());
        CIB_LOG(typename log_spec_t::flavor, log_spec_t::level, "flow.{}({})",
                stdx::ct_string_to_type<Type, sc::string_constant>(),
                stdx::ct_string_to_type<Name, sc::string_constant>());
    }
}
} // namespace detail

template <stdx::ct_string Type, stdx::ct_string Name,
          subgraph_identity Identity, typename Cond, typename F>
struct ct_node {
    using is_subgraph = void;
    using type_t =
        decltype(stdx::ct_string_to_type<Type, sc::string_constant>());

    using name_t =
        decltype(stdx::ct_string_to_type<Name, sc::string_constant>());

    constexpr static auto ct_name = Name;

    constexpr static auto identity = Identity;

    constexpr static auto condition = Cond{};

    constexpr operator rt_node() const {
        return rt_node{detail::run_func<Cond, F>,
                       detail::log_func<ct_node, Cond, Type, Name>};
    }

    constexpr auto operator*() const {
        if constexpr (Identity == subgraph_identity::REFERENCE) {
            return ct_node<Type, Name, subgraph_identity::VALUE, Cond, F>{};
        } else {
            return ct_node{};
        }
    }
};

namespace detail {
template <stdx::ct_string Type, stdx::ct_string Name, typename F>
[[nodiscard]] constexpr auto make_node() {
    return ct_node<Type, Name, subgraph_identity::REFERENCE,
                   cib::detail::always_condition_t, F>{};
}

constexpr auto empty_func = []() {};
} // namespace detail

template <stdx::ct_string Name, typename F>
    requires(stdx::is_function_object_v<F> and std::is_empty_v<F>)
[[nodiscard]] constexpr auto action(F const &) {
    return detail::make_node<"action", Name, F>();
}

template <stdx::ct_string Name> [[nodiscard]] constexpr auto action() {
    return action<Name>(cib::func_decl<Name>);
}

template <stdx::ct_string Name> [[nodiscard]] constexpr auto step() {
    return action<Name>(cib::func_decl<Name>);
}

template <stdx::ct_string Name> [[nodiscard]] constexpr auto milestone() {
    return detail::make_node<"milestone", Name, decltype(detail::empty_func)>();
}

inline namespace literals {
template <stdx::ct_string S> [[nodiscard]] constexpr auto operator""_action() {
    return action<S>();
}

template <stdx::ct_string S> [[nodiscard]] constexpr auto operator""_step() {
    return action<S>();
}

template <stdx::ct_string S>
[[nodiscard]] constexpr auto operator""_milestone() {
    return milestone<S>();
}
} // namespace literals

template <typename Cond, stdx::ct_string Type, stdx::ct_string Name,
          subgraph_identity Identity, typename NodeCond, typename F>
constexpr auto
make_runtime_conditional(Cond, ct_node<Type, Name, Identity, NodeCond, F>) {
    if constexpr (Identity == subgraph_identity::VALUE) {
        return ct_node<Type, Name, Identity, decltype(NodeCond{} and Cond{}),
                       F>{};
    } else {
        return ct_node<Type, Name, Identity, NodeCond, F>{};
    }
}

} // namespace flow
