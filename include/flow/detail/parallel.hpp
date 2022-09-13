#pragma once


#include <flow/detail/walk.hpp>


namespace flow::detail {
    template<
        typename NodeType,
        typename LhsType,
        typename RhsType>
    struct parallel {
        LhsType lhs;
        RhsType rhs;

        constexpr parallel(
            LhsType lhs,
            RhsType rhs
        )
            : lhs(lhs)
            , rhs(rhs)
        {}

        template<typename Callable>
        constexpr void walk(Callable c) const {
            detail::node_walk<NodeType, Callable, LhsType>(c, lhs);
            detail::node_walk<NodeType, Callable, RhsType>(c, rhs);
        }

        constexpr auto get_heads() const {
            return detail::get_heads<NodeType>(lhs) + detail::get_heads<NodeType>(rhs);
        }

        constexpr auto get_tails() const {
            return detail::get_tails<NodeType>(lhs) + detail::get_tails<NodeType>(rhs);
        }
    };
}





