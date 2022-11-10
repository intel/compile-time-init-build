#pragma once

#include <flow/detail/walk.hpp>

namespace flow::detail {
template <typename NodeType, typename LhsType, typename RhsType>
struct parallel {
    LhsType lhs;
    RhsType rhs;

    constexpr parallel(LhsType l, RhsType r) : lhs(l), rhs(r) {}

    template <typename Callable> constexpr void walk(Callable c) const {
        detail::node_walk<NodeType>(c, lhs);
        detail::node_walk<NodeType>(c, rhs);
    }

    constexpr auto get_heads() const {
        return detail::get_heads<NodeType>(lhs) +
               detail::get_heads<NodeType>(rhs);
    }

    constexpr auto get_tails() const {
        return detail::get_tails<NodeType>(lhs) +
               detail::get_tails<NodeType>(rhs);
    }
};
} // namespace flow::detail
