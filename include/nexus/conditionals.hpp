#pragma once

#include <nexus/detail/constexpr_conditional.hpp>
#include <nexus/detail/runtime_conditional.hpp>

#include <stdx/ct_string.hpp>

#include <type_traits>

namespace cib {
template <stdx::ct_string Name>
constexpr auto constexpr_condition = []<typename P>(P) {
    static_assert(std::is_default_constructible_v<P>);
    return detail::constexpr_condition<Name, P>{};
};

template <stdx::ct_string Name>
constexpr auto runtime_condition = []<typename P>(P) {
    static_assert(std::is_default_constructible_v<P>);
    return detail::runtime_condition<Name, P>{};
};
} // namespace cib
