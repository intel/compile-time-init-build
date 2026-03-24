#pragma once

#include <nexus/detail/extend.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/type_traits.hpp>

#include <type_traits>

namespace cib {
namespace detail {
template <typename T>
using maybe_funcptr_t =
    stdx::conditional_t<stdx::is_function_v<T>, std::decay_t<T>, T>;
}

template <typename Service, typename... Args>
[[nodiscard]] CONSTEVAL auto extend(Args const &...args) {
    return detail::type_extend<Service, detail::maybe_funcptr_t<Args>...>{
        args...};
}
template <stdx::ct_string Name, typename... Args>
[[nodiscard]] CONSTEVAL auto extend(Args const &...args) {
    return detail::name_extend<Name, detail::maybe_funcptr_t<Args>...>{args...};
}
} // namespace cib
