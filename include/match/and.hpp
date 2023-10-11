#pragma once

#include <match/bin_op.hpp>
#include <match/concepts.hpp>
#include <match/constant.hpp>
#include <match/simplify.hpp>
#include <sc/string_constant.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace match {
template <matcher, matcher> struct or_t;

template <matcher L, matcher R>
struct and_t : bin_op_t<and_t, decltype(" and "_sc), L, R> {
    [[nodiscard]] constexpr auto operator()(auto const &event) const -> bool {
        return this->lhs(event) and this->rhs(event);
    }

  private:
    [[nodiscard]] friend constexpr auto tag_invoke(simplify_t, and_t const &m) {
        return detail::simplify_bin_op<never_t, always_t, or_t>(m);
    }

    [[nodiscard]] friend constexpr auto tag_invoke(cost_t,
                                                   std::type_identity<and_t>)
        -> std::size_t {
        return cost(std::type_identity<L>{}) + cost(std::type_identity<R>{}) +
               1u;
    }
};
template <matcher L, matcher R> and_t(L, R) -> and_t<L, R>;

template <match::matcher L, match::matcher R>
[[nodiscard]] constexpr auto operator&(L const &lhs, R const &rhs)
    -> match::and_t<L, R> {
    return {lhs, rhs};
}

template <matcher... Ms> [[nodiscard]] constexpr auto all(Ms &&...ms) {
    return simplify((always & ... & std::forward<Ms>(ms)));
}
} // namespace match
