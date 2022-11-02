#pragma once

#include <sc/string_constant.hpp>

#include <type_traits>

namespace sc {
template <typename IntegralTypeT, IntegralTypeT ValueT,
          typename BaseTypeT = int, BaseTypeT BaseT = 10,
          bool UppercaseT = false,
          std::enable_if_t<std::is_integral<IntegralTypeT>::value, bool> = true>
[[nodiscard]] constexpr auto to_string_constant(
    std::integral_constant<IntegralTypeT, ValueT> const &,
    std::integral_constant<BaseTypeT, BaseT> const &base = int_<10>,
    std::integral_constant<bool, UppercaseT> const &uppercase = bool_<false>) {
    return create<detail::IntegralToString<IntegralTypeT, ValueT, BaseTypeT,
                                           BaseT, UppercaseT>>();
}

template <typename EnumTypeT, EnumTypeT ValueT,
          std::enable_if_t<std::is_enum<EnumTypeT>::value, bool> = true>
[[nodiscard]] constexpr auto
to_string_constant(std::integral_constant<EnumTypeT, ValueT> const &) {
    return create<detail::EnumToString<EnumTypeT, ValueT>>();
}

template <typename T>
[[nodiscard]] constexpr auto to_string_constant(type_name<T>) {
    return create<detail::TypeNameToString<T>>();
}

template <typename CharT, CharT... chars>
[[nodiscard]] constexpr auto
to_string_constant(string_constant<CharT, chars...> str) {
    return str;
}
} // namespace sc