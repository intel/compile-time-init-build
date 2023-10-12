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

    [[nodiscard]] friend constexpr auto tag_invoke(sum_of_products_t,
                                                   and_t const &m) {
        [[maybe_unused]] auto l = sum_of_products(m.lhs);
        [[maybe_unused]] auto r = sum_of_products(m.rhs);
        using LS = decltype(l);
        using RS = decltype(r);

        if constexpr (stdx::is_specialization_of_v<LS, or_t>) {
            auto lr = sum_of_products(
                and_t<typename LS::lhs_t, RS>{m.lhs.lhs, m.rhs});
            auto rr = sum_of_products(
                and_t<typename LS::rhs_t, RS>{m.lhs.rhs, m.rhs});
            return or_t{lr, rr};
        } else if constexpr (stdx::is_specialization_of_v<RS, or_t>) {
            auto ll = sum_of_products(
                and_t<LS, typename RS::lhs_t>{m.lhs, m.rhs.lhs});
            auto lr = sum_of_products(
                and_t<LS, typename RS::rhs_t>{m.lhs, m.rhs.rhs});
            return or_t{ll, lr};
        } else {
            return m;
        }
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
