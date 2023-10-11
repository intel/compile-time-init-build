#pragma once

#include <match/and.hpp>
#include <match/concepts.hpp>
#include <match/negate.hpp>
#include <match/not.hpp>
#include <match/or.hpp>
#include <match/simplify.hpp>

template <match::matcher L, match::matcher R>
[[nodiscard]] constexpr auto operator and(L const &lhs, R const &rhs)
    -> decltype(match::simplify(std::declval<match::and_t<L, R>>())) {
    return match::simplify(match::and_t{lhs, rhs});
}

template <match::matcher L, match::matcher R>
[[nodiscard]] constexpr auto operator or(L const &lhs, R const &rhs)
    -> decltype(match::simplify(std::declval<match::or_t<L, R>>())) {
    return match::simplify(match::or_t{lhs, rhs});
}

template <match::matcher M>
[[nodiscard]] constexpr auto operator not(M const &m)
    -> decltype(match::simplify(match::negate(std::declval<M>()))) {
    return match::simplify(match::negate(m));
}
