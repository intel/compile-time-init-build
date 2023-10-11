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
template <matcher, matcher> struct and_t;

template <matcher L, matcher R>
struct or_t : bin_op_t<or_t, decltype(" or "_sc), L, R> {
    [[nodiscard]] constexpr auto operator()(auto const &event) const -> bool {
        return this->lhs(event) or this->rhs(event);
    }

  private:
    [[nodiscard]] CONSTEVAL static auto annihilator() -> always_t { return {}; }

    [[nodiscard]] friend constexpr auto tag_invoke(simplify_t, or_t const &m) {
        return detail::simplify_bin_op<always_t, never_t, and_t>(m);
    }

    [[nodiscard]] friend constexpr auto tag_invoke(cost_t,
                                                   std::type_identity<or_t>)
        -> std::size_t {
        return cost(std::type_identity<L>{}) + cost(std::type_identity<R>{}) +
               1u;
    }
};
template <matcher L, matcher R> or_t(L, R) -> or_t<L, R>;

template <match::matcher L, match::matcher R>
[[nodiscard]] constexpr auto operator|(L const &lhs, R const &rhs)
    -> match::or_t<L, R> {
    return {lhs, rhs};
}

template <matcher... Ms> [[nodiscard]] constexpr auto any(Ms &&...ms) {
    return simplify((never | ... | std::forward<Ms>(ms)));
}
} // namespace match
