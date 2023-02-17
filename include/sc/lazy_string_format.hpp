#pragma once

#include <cib/tuple.hpp>
#include <cib/tuple_algorithms.hpp>
#include <sc/fwd.hpp>
#include <sc/string_constant.hpp>

namespace sc {
template <typename StringConstantT, typename ArgTupleT>
struct lazy_string_format {
    constexpr static StringConstantT str{};
    constexpr static bool has_args = ArgTupleT::size() > 0;
    ArgTupleT args{};

    constexpr lazy_string_format() = default;

    constexpr lazy_string_format(StringConstantT, ArgTupleT newArgs)
        : args{newArgs} {}
};

template <class CharT, CharT... charsLhs, CharT... charsRhs>
[[nodiscard]] constexpr auto operator==(
    lazy_string_format<string_constant<CharT, charsLhs...>, cib::tuple<>>,
    string_constant<CharT, charsRhs...>) noexcept {
    return bool_<false>;
}

template <class CharT, CharT... chars>
[[nodiscard]] constexpr auto
operator==(lazy_string_format<string_constant<CharT, chars...>, cib::tuple<>>,
           string_constant<CharT, chars...>) noexcept {
    return bool_<true>;
}

template <class CharT, CharT... charsLhs, typename ArgsTupleLhsT,
          CharT... charsRhs>
[[nodiscard]] constexpr auto operator==(
    lazy_string_format<string_constant<CharT, charsLhs...>, ArgsTupleLhsT>,
    string_constant<CharT, charsRhs...>) noexcept {
    return bool_<false>;
}

template <class CharT, CharT... charsLhs, CharT... charsRhs>
[[nodiscard]] constexpr auto operator!=(
    lazy_string_format<string_constant<CharT, charsLhs...>, cib::tuple<>>,
    string_constant<CharT, charsRhs...>) noexcept {
    return bool_<true>;
}

template <class CharT, CharT... chars>
[[nodiscard]] constexpr auto
operator!=(lazy_string_format<string_constant<CharT, chars...>, cib::tuple<>>,
           string_constant<CharT, chars...>) noexcept {
    return bool_<false>;
}

template <class CharT, CharT... charsLhs, typename ArgsTupleLhsT,
          CharT... charsRhs>
[[nodiscard]] constexpr auto operator!=(
    lazy_string_format<string_constant<CharT, charsLhs...>, ArgsTupleLhsT>,
    string_constant<CharT, charsRhs...>) noexcept {
    return bool_<true>;
}

template <class CharT, CharT... charsLhs, typename ArgsTupleLhsT,
          CharT... charsRhs, typename ArgsTupleRhsT>
[[nodiscard]] constexpr auto operator==(
    lazy_string_format<string_constant<CharT, charsLhs...>, ArgsTupleLhsT> lhs,
    lazy_string_format<string_constant<CharT, charsRhs...>, ArgsTupleRhsT>
        rhs) noexcept -> bool {
    return (lhs.str == rhs.str) && (lhs.args == rhs.args);
}

template <class CharT, CharT... charsLhs, typename ArgsTupleLhsT,
          CharT... charsRhs, typename ArgsTupleRhsT>
[[nodiscard]] constexpr auto operator!=(
    lazy_string_format<string_constant<CharT, charsLhs...>, ArgsTupleLhsT> lhs,
    lazy_string_format<string_constant<CharT, charsRhs...>, ArgsTupleRhsT>
        rhs) noexcept -> bool {
    return !(lhs == rhs);
}

template <typename StringConstantLhsT, typename TupleArgsLhsT,
          typename StringConstantRhsT, typename TupleArgsRhsT>
[[nodiscard]] constexpr auto
operator+(lazy_string_format<StringConstantLhsT, TupleArgsLhsT> lhs,
          lazy_string_format<StringConstantRhsT, TupleArgsRhsT> rhs) noexcept {
    return lazy_string_format{lhs.str + rhs.str,
                              cib::tuple_cat(lhs.args, rhs.args)};
}

template <typename StringConstantLhsT, typename TupleArgsLhsT, typename CharT,
          CharT... chars>
[[nodiscard]] constexpr auto
operator+(lazy_string_format<StringConstantLhsT, TupleArgsLhsT> lhs,
          string_constant<CharT, chars...> rhs) noexcept {
    return lazy_string_format{lhs.str + rhs, lhs.args};
}

template <typename CharT, CharT... chars, typename StringConstantRhsT,
          typename TupleArgsRhsT>
[[nodiscard]] constexpr auto
operator+(string_constant<CharT, chars...> lhs,
          lazy_string_format<StringConstantRhsT, TupleArgsRhsT> rhs) noexcept {
    return lazy_string_format{lhs + rhs.str, rhs.args};
}

template <typename StringConstantLhsT, typename StringConstantRhsT>
[[nodiscard]] constexpr auto
operator+(lazy_string_format<StringConstantLhsT, cib::tuple<>> lhs,
          lazy_string_format<StringConstantRhsT, cib::tuple<>> rhs) noexcept {
    return lhs.str + rhs.str;
}

template <typename StringConstantLhsT, typename CharT, CharT... chars>
[[nodiscard]] constexpr auto
operator+(lazy_string_format<StringConstantLhsT, cib::tuple<>> lhs,
          string_constant<CharT, chars...> rhs) noexcept {
    return lhs.str + rhs;
}

template <typename CharT, CharT... chars, typename StringConstantRhsT>
[[nodiscard]] constexpr auto
operator+(string_constant<CharT, chars...> lhs,
          lazy_string_format<StringConstantRhsT, cib::tuple<>> rhs) noexcept {
    return lhs + rhs.str;
}
} // namespace sc
