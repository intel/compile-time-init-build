#pragma once

#include <match/bin_op.hpp>
#include <match/concepts.hpp>
#include <match/constant.hpp>
#include <match/simplify.hpp>
#include <match/sum_of_products.hpp>
#include <sc/string_constant.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace match {
template <matcher, matcher> struct and_t;

template <matcher L, matcher R> struct or_t : bin_op_t<or_t, " or ", L, R> {
    [[nodiscard]] constexpr auto operator()(auto const &event) const -> bool {
        return this->lhs(event) or this->rhs(event);
    }

  private:
    [[nodiscard]] friend constexpr auto tag_invoke(simplify_t, or_t const &m) {
        auto l = simplify(m.lhs);
        auto r = simplify(m.rhs);

        if constexpr (implies(l, r)) {
            return r;
        } else if constexpr (implies(r, l)) {
            return l;
        } else if constexpr (implies(negate(l), r) or implies(negate(r), l)) {
            return always;
        } else {
            return detail::de_morgan<or_t, and_t>(l, r);
        }
    }

    [[nodiscard]] friend constexpr auto tag_invoke(cost_t,
                                                   std::type_identity<or_t>)
        -> std::size_t {
        return cost(std::type_identity<L>{}) + cost(std::type_identity<R>{}) +
               1u;
    }

    [[nodiscard]] friend constexpr auto tag_invoke(sum_of_products_t,
                                                   or_t const &m) {
        auto l = sum_of_products(m.lhs);
        auto r = sum_of_products(m.rhs);
        using LS = decltype(l);
        using RS = decltype(r);
        return or_t<LS, RS>{l, r};
    }

    template <matcher M>
    [[nodiscard]] friend constexpr auto tag_invoke(implies_t, M const &m,
                                                   or_t const &o) -> bool {
        return implies(m, o.lhs) or implies(m, o.rhs);
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
