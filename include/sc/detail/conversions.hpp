#pragma once

#include <sc/detail/string_meta.hpp>

#include <array>
#include <cstddef>
#include <string_view>
#include <type_traits>

namespace sc::detail {
template <typename CharT, int SizeT> struct static_string {
    std::array<CharT, SizeT> data;
    std::size_t pos;
    std::size_t count;

    constexpr static_string() : data{}, pos{}, count{} {}

    constexpr operator std::basic_string_view<CharT>() const {
        return std::string_view{data.data() + pos, count};
    }
};

template <typename IntegralTypeT, typename BaseTypeT>
[[nodiscard]] constexpr static_string<char, 65>
integral_to_string(IntegralTypeT value, BaseTypeT base, bool uppercase) {
    constexpr std::size_t MAX_LENGTH = 65;

    bool const negative = std::is_signed_v<IntegralTypeT> && (value < 0);
    char const ext_char_start = uppercase ? 'A' : 'a';

    value = negative ? -value : value;
    auto digit = MAX_LENGTH;

    static_string<char, MAX_LENGTH> ret{};

    if (value == 0) {
        digit = digit - 1;
        ret.data[digit] = '0';
    } else {
        while (value > 0) {
            auto const digit_value = (value % base);
            auto const digit_char = [=]() {
                if (digit_value > 9) {
                    return (digit_value - 10) + ext_char_start;
                } else {
                    return digit_value + '0';
                }
            }();

            digit -= 1;
            ret.data[digit] = static_cast<char>(digit_char);
            value /= base;
        }
    }

    if (negative) {
        digit -= 1;
        ret.data[digit] = '-';
    }

    ret.pos = digit;
    ret.count = MAX_LENGTH - digit;

    return ret;
}

template <typename IntegralTypeT, IntegralTypeT ValueT, typename BaseTypeT,
          BaseTypeT BaseT, bool UppercaseT>
struct IntegralToString {
    constexpr static int MAX_LENGTH = 65;

    constexpr static static_string<char, MAX_LENGTH> intermediate =
        integral_to_string(ValueT, BaseT, UppercaseT);

    constexpr static std::basic_string_view<char> value = intermediate;
};

template <typename Tag> constexpr static std::string_view type_as_string() {
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
constexpr static std::basic_string_view<char> enum_as_string() {
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
