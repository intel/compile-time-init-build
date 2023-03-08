#pragma once

#include <sc/detail/conversions.hpp>
#include <sc/string_constant.hpp>

#include <concepts>
#include <type_traits>

namespace sc {
template <std::integral T, T Value, T Base = T{10}, bool Uppercase = false>
[[nodiscard]] constexpr auto to_string_constant(
    std::integral_constant<T, Value> const &,
    [[maybe_unused]] std::integral_constant<T, Base> const &base = {},
    [[maybe_unused]] std::bool_constant<Uppercase> const &uppercase = {}) {
    return detail::create<
        detail::IntegralToString<T, Value, Base, Uppercase>>();
}

template <typename EnumTypeT, EnumTypeT ValueT,
          std::enable_if_t<std::is_enum_v<EnumTypeT>, bool> = true>
[[nodiscard]] constexpr auto
to_string_constant(std::integral_constant<EnumTypeT, ValueT> const &) {
    return detail::create<detail::EnumToString<EnumTypeT, ValueT>>();
}

template <typename T>
[[nodiscard]] constexpr auto to_string_constant(type_name<T>) {
    return detail::create<detail::TypeNameToString<T>>();
}

template <typename CharT, CharT... chars>
[[nodiscard]] constexpr auto
to_string_constant(string_constant<CharT, chars...> str) {
    return str;
}
} // namespace sc
