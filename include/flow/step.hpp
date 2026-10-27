#pragma once

#include <flow/dsl/subgraph_identity.hpp>
#include <flow/log.hpp>
#include <nexus/detail/runtime_conditional.hpp>
#include <nexus/func_decl.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/type_traits.hpp>

#include <type_traits>

namespace flow {
template <auto Type, auto Name, dsl::subgraph_identity Identity, typename Cond,
          typename F>
struct ct_node : stdx::with_shrunk_name<Name> {
    using is_subgraph = void;
    using func_t = F;
    using cond_t = Cond;

    constexpr static auto ct_type = Type;
    constexpr static auto identity = Identity;
    constexpr static auto condition = cond_t{};

    constexpr auto operator*() const {
        if constexpr (Identity == dsl::subgraph_identity::REFERENCE) {
            return ct_node<Type, Name, dsl::subgraph_identity::VALUE, cond_t,
                           F>{};
        } else {
            return ct_node{};
        }
    }
};

namespace detail {
template <stdx::ct_string Type, stdx::ct_string Name, typename F>
[[nodiscard]] consteval auto make_node() {
    return ct_node<stdx::shrink<Type>(), stdx::shrink<Name>(),
                   dsl::subgraph_identity::REFERENCE,
                   cib::detail::always_condition_t, F>{};
}

constexpr auto empty_func = []() -> void {};

template <typename Node>
constexpr bool is_milestone =
    stdx::expand(Node::ct_type) == stdx::ct_string{"milestone"};
} // namespace detail

template <stdx::ct_string Name, typename F>
    requires(stdx::is_function_object_v<F> and std::is_empty_v<F>)
[[nodiscard]] consteval auto action(F const &) {
    return detail::make_node<"action", Name, F>();
}

template <stdx::ct_string Name> [[nodiscard]] consteval auto action() {
    return action<Name>(cib::func_decl<Name>);
}

template <stdx::ct_string Name> [[nodiscard]] consteval auto step() {
    return action<Name>(cib::func_decl<Name>);
}

template <stdx::ct_string Name> [[nodiscard]] consteval auto milestone() {
    return detail::make_node<"milestone", Name, decltype(detail::empty_func)>();
}

inline namespace literals {
template <stdx::ct_string S> [[nodiscard]] consteval auto operator""_action() {
    return action<S>();
}

template <stdx::ct_string S> [[nodiscard]] consteval auto operator""_step() {
    return action<S>();
}

template <stdx::ct_string S>
[[nodiscard]] consteval auto operator""_milestone() {
    return milestone<S>();
}
} // namespace literals

template <typename Cond, auto Type, auto Name, dsl::subgraph_identity Identity,
          typename NodeCond, typename F>
constexpr auto
make_runtime_conditional(Cond, ct_node<Type, Name, Identity, NodeCond, F>) {
    if constexpr (Identity == dsl::subgraph_identity::VALUE) {
        return ct_node<Type, Name, Identity, decltype(NodeCond{} and Cond{}),
                       F>{};
    } else {
        return ct_node<Type, Name, Identity, NodeCond, F>{};
    }
}

} // namespace flow
