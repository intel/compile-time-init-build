#pragma once

#include <stdx/compiler.hpp>
#include <stdx/ct_string.hpp>

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
template <stdx::ct_string S> CONSTEVAL auto operator""_sc() {
    return stdx::ct_string_to_type<S, string_constant>();
}
} // namespace literals

template <typename T>
concept sc_like = requires(T const &t) {
    t.apply([]<typename StringType>(StringType, auto const &...) {});
};
} // namespace sc
using sc::literals::operator""_sc;
