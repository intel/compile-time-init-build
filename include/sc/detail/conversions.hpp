#pragma once

#include <sc/detail/string_meta.hpp>

#include <array>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <string_view>
#include <type_traits>

namespace sc::detail {
template <typename CharT, int SizeT> struct static_string {
    std::array<CharT, SizeT> data{};
    std::size_t size{};

    constexpr explicit operator std::basic_string_view<CharT>() const {
        return std::basic_string_view<CharT>{
            std::prev(data.end(), static_cast<int>(size)), size};
    }
};

[[nodiscard]] constexpr auto to_chars(auto value, auto last, auto base,
                                      auto op) noexcept {
    while (value != 0) {
        *--last = static_cast<char>(op(value % base));
        value /= base;
    }
    return last;
}

template <std::integral T>
[[nodiscard]] constexpr auto integral_to_string(T value, T base, bool uppercase)
    -> static_string<char, 65> {
    constexpr std::size_t MAX_LENGTH = 65;
    auto const ext_char_start = static_cast<T>(uppercase ? 'A' : 'a');
    static_string<char, MAX_LENGTH> ret{};
    auto it = ret.data.end();

    if (value == 0) {
        *--it = '0';
    } else {
        if constexpr (std::signed_integral<T>) {
            if (value < T{}) {
                it = to_chars(value, it, base, [=](auto v) {
                    if (v < -9) {
                        return 10 - v + ext_char_start;
                    }
                    return '0' - v;
                });
                *--it = '-';
                ret.size =
                    static_cast<std::size_t>(std::distance(it, ret.data.end()));
                return ret;
            }
        }
        it = to_chars(value, it, base, [=](auto v) {
            if (v > 9) {
                return v - 10 + ext_char_start;
            }
            return '0' + v;
        });
    }
    ret.size = static_cast<std::size_t>(std::distance(it, ret.data.end()));
    return ret;
}

template <typename T, T Value, T Base, bool Uppercase> struct IntegralToString {
    constexpr static int MAX_LENGTH = 65;

    constexpr static static_string<char, MAX_LENGTH> intermediate =
        integral_to_string(Value, Base, Uppercase);

    constexpr static std::string_view value =
        static_cast<std::string_view>(intermediate);
};

template <typename Tag>
constexpr static auto type_as_string() -> std::string_view {
#if defined(__clang__)
    constexpr std::string_view function_name = __PRETTY_FUNCTION__;
    constexpr auto rhs = function_name.size() - 2;

#elif defined(__GNUC__) || defined(__GNUG__)
    constexpr std::string_view function_name = __PRETTY_FUNCTION__;
    constexpr auto rhs = function_name.size() - 51;

#else
    static_assert(false, "Unknown compiler, can't build type name.");
#endif

    constexpr auto lhs = [&]() -> std::string_view::size_type {
        for (auto i = std::size_t{}; i < function_name.size(); i++) {
            if (function_name[i] == '=') {
                return i + 2;
            }
        }
        return 0;
    }();

    return function_name.substr(lhs, rhs - lhs + 1);
}

template <auto Value>
constexpr static auto enum_as_string() -> std::basic_string_view<char> {
#if defined(__clang__)
    constexpr std::string_view value_string = __PRETTY_FUNCTION__;
    constexpr auto rhs = value_string.size() - 2;

#elif defined(__GNUC__) || defined(__GNUG__)
    constexpr std::string_view value_string = __PRETTY_FUNCTION__;
    constexpr auto rhs = value_string.size() - 2;

#else
    static_assert(false, "Unknown compiler, can't build type name.");
#endif

    constexpr auto lhs = [&]() -> std::string_view::size_type {
        if (const auto colon_pos = value_string.find_last_of(':');
            colon_pos != std::string_view::npos) {
            return colon_pos + 1;
        }
        return 0;
    }();

    return value_string.substr(lhs, rhs - lhs + 1);
}

template <typename EnumTypeT, EnumTypeT ValueT> struct EnumToString {
    struct PrettyFunction {
        constexpr static std::string_view value = enum_as_string<ValueT>();
    };

    constexpr static auto value = create<PrettyFunction>();
};

template <typename T> struct TypeNameToString {
    struct PrettyFunction {
        constexpr static std::string_view value = type_as_string<T>();
    };

    constexpr static auto value = create<PrettyFunction>();
};
} // namespace sc::detail
