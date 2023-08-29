#pragma once

#include <type_traits>

namespace sc {
template <int value> constexpr static std::integral_constant<int, value> int_{};

template <unsigned int value>
constexpr static std::integral_constant<unsigned int, value> uint_{};

template <bool value>
constexpr static std::integral_constant<bool, value> bool_{};

template <char value>
constexpr static std::integral_constant<char, value> char_{};

template <auto enumValue>
constexpr static std::integral_constant<decltype(enumValue), enumValue> enum_{};

template <typename T> struct type_name {
    constexpr explicit type_name(T) {}
    constexpr type_name() = default;
};

template <typename T> constexpr static type_name<T> type_{};

template <typename CharT, CharT... chars> struct string_constant;
} // namespace sc

template <class T, T... chars>
constexpr auto operator""_sc() -> sc::string_constant<T, chars...> {
    return {};
}
