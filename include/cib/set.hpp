#include "detail/set_details.hpp"

#ifndef CIB_SET_HPP
#define CIB_SET_HPP


namespace cib {
    template<
        typename LhsTuple,
        typename RhsTuple,
        typename MetaFunc = self_type>
    [[nodiscard]] constexpr auto set_union(
        LhsTuple lhs,
        RhsTuple rhs,
        MetaFunc meta_func = self_type{}
    ) {
        return set_operation_impl(detail::set_operation_algorithm<true, true, true>{}, meta_func, lhs, rhs);
    }

    template<
        typename LhsTuple,
        typename RhsTuple,
        typename MetaFunc = self_type>
    [[nodiscard]] constexpr auto set_intersection(
        LhsTuple lhs,
        RhsTuple rhs,
        MetaFunc meta_func = self_type{}
    ) {
        return set_operation_impl(detail::set_operation_algorithm<false, true, false>{}, meta_func, lhs, rhs);
    }

    template<
        typename LhsTuple,
        typename RhsTuple,
        typename MetaFunc = self_type>
    [[nodiscard]] constexpr auto set_difference(
        LhsTuple lhs,
        RhsTuple rhs,
        MetaFunc meta_func = self_type{}
    ) {
        return set_operation_impl(detail::set_operation_algorithm<true, false, false>{}, meta_func, lhs, rhs);
    }

    template<
        typename LhsTuple,
        typename RhsTuple,
        typename MetaFunc = self_type>
    [[nodiscard]] constexpr auto set_symmetric_difference(
        LhsTuple lhs,
        RhsTuple rhs,
        MetaFunc meta_func = self_type{}
    ) {
        return set_operation_impl(detail::set_operation_algorithm<true, false, true>{}, meta_func, lhs, rhs);
    }
}

#endif //CIB_SET_HPP
