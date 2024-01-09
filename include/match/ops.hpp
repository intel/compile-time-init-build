#pragma once

#include <match/and.hpp>
#include <match/concepts.hpp>
#include <match/implies.hpp>
#include <match/negate.hpp>
#include <match/not.hpp>
#include <match/or.hpp>
#include <match/simplify.hpp>

#include <compare>
#include <utility>

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

template <match::matcher L, match::matcher R>
[[nodiscard]] constexpr auto operator<=>(L const &lhs, R const &rhs)
    -> std::partial_ordering {
    auto const l = match::simplify(lhs);
    auto const r = match::simplify(rhs);
    auto const x = match::implies(l, r);
    auto const y = match::implies(r, l);
    if (not x and not y) {
        return std::partial_ordering::unordered;
    }
    return y <=> x;
}
