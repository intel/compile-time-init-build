#pragma once

#include <concepts>
#include <type_traits>

namespace sc {
template <std::signed_integral auto value>
constexpr static std::integral_constant<decltype(value), value> int_{};

template <std::unsigned_integral auto value>
constexpr static std::integral_constant<decltype(value), value> uint_{};

template <bool value>
constexpr static std::integral_constant<bool, value> bool_{};

template <char value>
constexpr static std::integral_constant<char, value> char_{};

template <auto enumValue>
    requires std::is_enum_v<decltype(enumValue)>
constexpr static std::integral_constant<decltype(enumValue), enumValue> enum_{};

template <typename T> struct type_name {
    constexpr explicit type_name(T) {}
    constexpr type_name() = default;
};

template <typename T> constexpr static type_name<T> type_{};

template <typename CharT, CharT... chars> struct string_constant;

inline namespace literals {
template <class T, T... chars>
constexpr auto operator""_sc() -> sc::string_constant<T, chars...> {
    return {};
}
} // namespace literals
} // namespace sc
using sc::literals::operator""_sc;
