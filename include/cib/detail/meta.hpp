#pragma once

#include <cib/detail/compiler.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace cib::detail {
template <int value>
CIB_CONSTEXPR static auto int_ = std::integral_constant<int, value>{};

template <std::size_t value>
CIB_CONSTEXPR static auto size_ = std::integral_constant<std::size_t, value>{};

template <typename Lhs, typename Rhs>
CIB_CONSTEXPR static auto is_same_v =
    std::is_same_v<std::remove_cv_t<std::remove_reference_t<Lhs>>,
                   std::remove_cv_t<std::remove_reference_t<Rhs>>>;

/**
 * Perform an operation on each element of an integral sequence.
 *
 * @param operation
 *      The operation to perform. Must be a callable that accepts a single
 * parameter.
 */
template <typename IntegralType, IntegralType... Indices, typename CallableType>
CIB_CONSTEXPR inline static void
for_each([[maybe_unused]] std::integer_sequence<IntegralType, Indices...> const
             &sequence,
         CallableType const &operation) {
    (operation(std::integral_constant<IntegralType, Indices>{}), ...);
}

/**
 * Perform an operation on each element of an integral sequence from 0 to
 * num_elements.
 *
 * @param operation
 *      The operation to perform. Must be a callable that accepts a single
 * parameter.
 */
template <typename IntegralType, IntegralType NumElements,
          typename CallableType>
CIB_CONSTEXPR inline void for_each(
    [[maybe_unused]] std::integral_constant<IntegralType, NumElements> const
        &num_elements,
    CallableType const &operation) {
    CIB_CONSTEXPR auto seq =
        std::make_integer_sequence<IntegralType, NumElements>{};
    for_each(seq, operation);
}
} // namespace cib::detail
