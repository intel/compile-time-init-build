#pragma once

#include <match/concepts.hpp>
#include <match/constant.hpp>
#include <match/cost.hpp>
#include <match/negate.hpp>
#include <match/simplify.hpp>
#include <match/sum_of_products.hpp>
#include <sc/format.hpp>
#include <sc/string_constant.hpp>

#include <stdx/type_traits.hpp>

#include <type_traits>

namespace match {
template <matcher, matcher> struct and_t;
template <matcher, matcher> struct or_t;

template <matcher M> struct not_t {
    using is_matcher = void;
    [[no_unique_address]] M m;

    [[nodiscard]] constexpr auto operator()(auto const &event) const -> bool {
        return not m(event);
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("not ({})"_sc, m.describe());
    }

    [[nodiscard]] constexpr auto describe_match(auto const &event) const {
        return format("not ({})"_sc, m.describe_match(event));
    }

  private:
    [[nodiscard]] friend constexpr auto tag_invoke(simplify_t, not_t const &n) {
        if constexpr (std::is_same_v<M, always_t>) {
            return never;
        } else if constexpr (std::is_same_v<M, never_t>) {
            return always;
        } else {
            return negate(simplify(n.m));
        }
    }

    [[nodiscard]] friend constexpr auto tag_invoke(cost_t,
                                                   std::type_identity<not_t>)
        -> std::size_t {
        return cost(std::type_identity<M>{}) + 1u;
    }

    [[nodiscard]] friend constexpr auto tag_invoke(negate_t, not_t const &n)
        -> M {
        return n.m;
    }

    [[nodiscard]] friend constexpr auto tag_invoke(sum_of_products_t,
                                                   not_t const &n) {
        if constexpr (stdx::is_specialization_of_v<M, and_t>) {
            return or_t{sum_of_products(negate(n.m.lhs)),
                        sum_of_products(negate(n.m.rhs))};
        } else if constexpr (stdx::is_specialization_of_v<M, or_t>) {
            return sum_of_products(and_t{sum_of_products(negate(n.m.lhs)),
                                         sum_of_products(negate(n.m.rhs))});
        } else {
            return n;
        }
    }
};
template <matcher M> not_t(M) -> not_t<M>;
} // namespace match
