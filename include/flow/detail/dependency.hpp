#pragma once

#include <flow/detail/walk.hpp>

namespace flow::detail {
template <typename LhsType, typename RhsType> struct dependency {
    LhsType lhs;
    RhsType rhs;

    constexpr dependency(LhsType l, RhsType r) : lhs(l), rhs(r) {}

    template <typename Callable> constexpr void walk(Callable c) const {
        detail::node_walk(c, lhs);
        detail::node_walk(c, rhs);

        for (auto tail : detail::get_tails(lhs)) {
            for (auto head : detail::get_heads(rhs)) {
                c(tail, head);
            }
        }
    }

    constexpr auto get_heads() const { return detail::get_heads(lhs); }

    constexpr auto get_tails() const { return detail::get_tails(rhs); }
};
} // namespace flow::detail
