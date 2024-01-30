#pragma once

#include <match/bin_op.hpp>
#include <match/concepts.hpp>
#include <match/constant.hpp>
#include <match/implies.hpp>
#include <match/simplify.hpp>
#include <match/sum_of_products.hpp>
#include <sc/string_constant.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace match {
template <matcher, matcher> struct or_t;

template <matcher L, matcher R> struct and_t : bin_op_t<and_t, " and ", L, R> {
    [[nodiscard]] constexpr auto operator()(auto const &event) const -> bool {
        return this->lhs(event) and this->rhs(event);
    }

  private:
    [[nodiscard]] friend constexpr auto tag_invoke(simplify_t, and_t const &m) {
        auto l = simplify(m.lhs);
        auto r = simplify(m.rhs);

        if constexpr (implies(l, r)) {
            return l;
        } else if constexpr (implies(r, l)) {
            return r;
        } else if constexpr (implies(l, negate(r)) or implies(r, negate(l))) {
            return never;
        } else {
            return detail::de_morgan<and_t, or_t>(l, r);
        }
    }

    [[nodiscard]] friend constexpr auto tag_invoke(cost_t,
                                                   std::type_identity<and_t>)
        -> std::size_t {
        return cost(std::type_identity<L>{}) + cost(std::type_identity<R>{}) +
               1u;
    }

    [[nodiscard]] friend constexpr auto tag_invoke(sum_of_products_t,
                                                   and_t const &m) {
        auto l = sum_of_products(m.lhs);
        auto r = sum_of_products(m.rhs);
        using LS = decltype(l);
        using RS = decltype(r);

        if constexpr (stdx::is_specialization_of_v<LS, or_t>) {
            auto lr = sum_of_products(and_t<typename LS::lhs_t, RS>{l.lhs, r});
            auto rr = sum_of_products(and_t<typename LS::rhs_t, RS>{l.rhs, r});
            return or_t{lr, rr};
        } else if constexpr (stdx::is_specialization_of_v<RS, or_t>) {
            auto ll = sum_of_products(and_t<LS, typename RS::lhs_t>{l, r.lhs});
            auto lr = sum_of_products(and_t<LS, typename RS::rhs_t>{l, r.rhs});
            return or_t{ll, lr};
        } else {
            return and_t<LS, RS>{l, r};
        }
    }

    template <matcher M>
        requires(not stdx::is_specialization_of_v<M, or_t>)
    [[nodiscard]] friend constexpr auto tag_invoke(implies_t, and_t const &a,
                                                   M const &m) -> bool {
        return implies(a.lhs, m) or implies(a.rhs, m);
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
