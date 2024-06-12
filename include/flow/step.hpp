#pragma once

#include <cib/func_decl.hpp>
#include <flow/common.hpp>
#include <log/log.hpp>
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

template <stdx::ct_string Name> struct ct_node : rt_node {
    using is_node = void;
    using name_t =
        decltype(stdx::ct_string_to_type<Name, sc::string_constant>());
};

namespace detail {
template <stdx::ct_string Name, stdx::ct_string Type, typename F>
[[nodiscard]] constexpr auto make_node() {
    return ct_node<Name>{
        {.run = F{}, .log_name = [] {
             CIB_TRACE("flow.{}({})",
                       stdx::ct_string_to_type<Type, sc::string_constant>(),
                       stdx::ct_string_to_type<Name, sc::string_constant>());
         }}};
}
} // namespace detail

template <stdx::ct_string Name, typename F>
    requires(stdx::is_function_object_v<F> and std::is_empty_v<F>)
[[nodiscard]] constexpr auto action(F const &) {
    return detail::make_node<Name, "action", F>();
}

template <stdx::ct_string Name> [[nodiscard]] constexpr auto action() {
    return action<Name>(cib::func_decl<Name>);
}

template <stdx::ct_string Name> [[nodiscard]] constexpr auto step() {
    return action<Name>(cib::func_decl<Name>);
}

template <stdx::ct_string Name> [[nodiscard]] constexpr auto milestone() {
    return detail::make_node<Name, "milestone", decltype([] {})>();
}

inline namespace literals {
template <class T, T... Cs> [[nodiscard]] constexpr auto operator""_action() {
    constexpr auto S = stdx::ct_string<sizeof...(Cs) + 1U>{{Cs..., 0}};
    return action<S>();
}

template <class T, T... Cs> [[nodiscard]] constexpr auto operator""_step() {
    constexpr auto S = stdx::ct_string<sizeof...(Cs) + 1U>{{Cs..., 0}};
    return action<S>();
}

template <class T, T... Cs>
[[nodiscard]] constexpr auto operator""_milestone() {
    constexpr auto S = stdx::ct_string<sizeof...(Cs) + 1U>{{Cs..., 0}};
    return milestone<S>();
}
} // namespace literals
} // namespace flow
