#pragma once


#include <flow/detail/walk.hpp>


namespace flow::detail {
    template<
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
            detail::node_walk(c, lhs);
            detail::node_walk(c, rhs);
        }

        constexpr auto get_heads() const {
            return detail::get_heads(lhs) + detail::get_heads(rhs);
        }

        constexpr auto get_tails() const {
            return detail::get_tails(lhs) + detail::get_tails(rhs);
        }
    };
}





