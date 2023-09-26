#pragma once

#include <sc/fwd.hpp>

#include <stdx/tuple_algorithms.hpp>

namespace sc {
template <typename StringConstant, typename ArgTuple>
struct lazy_string_format {
    constexpr static StringConstant str{};
    ArgTuple args{};

    constexpr lazy_string_format() = default;
    constexpr lazy_string_format(StringConstant, ArgTuple newArgs)
        : args{newArgs} {}

    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
    template <typename F> constexpr auto apply(F &&f) const {
        return args.apply(
            [&](auto const &...as) { return std::forward<F>(f)(str, as...); });
    }
};

template <class CharT, CharT... charsLhs, typename ArgsTupleLhs,
          CharT... charsRhs, typename ArgsTupleRhs>
[[nodiscard]] constexpr auto operator==(
    lazy_string_format<string_constant<CharT, charsLhs...>, ArgsTupleLhs> lhs,
    lazy_string_format<string_constant<CharT, charsRhs...>, ArgsTupleRhs>
        rhs) noexcept -> bool {
    return (lhs.str == rhs.str) && (lhs.args == rhs.args);
}

template <typename StringConstantLhs, typename TupleArgsLhs,
          typename StringConstantRhs, typename TupleArgsRhs>
[[nodiscard]] constexpr auto
operator+(lazy_string_format<StringConstantLhs, TupleArgsLhs> lhs,
          lazy_string_format<StringConstantRhs, TupleArgsRhs> rhs) noexcept {
    return lazy_string_format{lhs.str + rhs.str,
                              stdx::tuple_cat(lhs.args, rhs.args)};
}

template <typename StringConstantLhs, typename TupleArgsLhs, typename CharT,
          CharT... chars>
[[nodiscard]] constexpr auto
operator+(lazy_string_format<StringConstantLhs, TupleArgsLhs> lhs,
          string_constant<CharT, chars...> rhs) noexcept {
    return lazy_string_format{lhs.str + rhs, lhs.args};
}

template <typename CharT, CharT... chars, typename StringConstantRhs,
          typename TupleArgsRhs>
[[nodiscard]] constexpr auto
operator+(string_constant<CharT, chars...> lhs,
          lazy_string_format<StringConstantRhs, TupleArgsRhs> rhs) noexcept {
    return lazy_string_format{lhs + rhs.str, rhs.args};
}
} // namespace sc
