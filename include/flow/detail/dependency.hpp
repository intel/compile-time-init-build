#pragma once

#include <flow/detail/walk.hpp>

namespace flow::detail {
template <typename NodeType, typename LhsType, typename RhsType>
struct dependency {
    LhsType lhs;
    RhsType rhs;

    constexpr dependency(LhsType l, RhsType r) : lhs(l), rhs(r) {}

    template <typename Callable> constexpr void walk(Callable c) const {
        detail::node_walk<NodeType>(c, lhs);
        detail::node_walk<NodeType>(c, rhs);

        for (auto tail : detail::get_tails<NodeType>(lhs)) {
            for (auto head : detail::get_heads<NodeType>(rhs)) {
                c(tail, head);
            }
        }
    }

    constexpr auto get_heads() const {
        return detail::get_heads<NodeType>(lhs);
    }

    constexpr auto get_tails() const {
        return detail::get_tails<NodeType>(rhs);
    }
};
} // namespace flow::detail
