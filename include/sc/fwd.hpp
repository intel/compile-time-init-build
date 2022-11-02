#pragma once

#include <type_traits>

namespace sc {
using size_type = int;
using difference_type = int;

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
    constexpr type_name(T) {}
    constexpr type_name() {}
};

template <typename T> constexpr static type_name<T> type_{T{}};

template <typename CharT, CharT... chars> struct string_constant;
} // namespace sc

template <class T, T... chars> constexpr auto operator""_sc() {
    return sc::string_constant<T, chars...>{};
}
