#include "compiler.hpp"
#include "tuple.hpp"

#include <tuple>
#include <type_traits>


#ifndef COMPILE_TIME_INIT_BUILD_META_HPP
#define COMPILE_TIME_INIT_BUILD_META_HPP


namespace cib::detail {
    template<int value>
    CIB_CONSTEXPR static auto int_ = std::integral_constant<int, value>{};

    template<typename Lhs, typename Rhs>
    CIB_CONSTEXPR static auto is_same_v =
        std::is_same_v<
            std::remove_cv_t<std::remove_reference_t<Lhs>>,
            std::remove_cv_t<std::remove_reference_t<Rhs>>
        >;

    /**
     * Used by fold_right to leverage c++17 fold expressions with arbitrary
     * callables.
     *
     * @tparam ValueType
     *      The type of the element from the value pack.
     *
     * @tparam CallableType
     *      A callable that takes two arguments, current element to be
     *      processed and the fold state.
     */
    template<
        typename ValueType,
        typename CallableType>
    struct fold_helper {
        ValueType const & element_;
        CallableType const & operation_;

        CIB_CONSTEXPR fold_helper(
            ValueType const & element,
            CallableType const & operation
        )
            : element_{element}
            , operation_{operation}
        {}

        template<typename StateType>
        [[nodiscard]] CIB_CONSTEXPR inline auto operator+(
            StateType const & state
        ) const {
            return operation_(element_, state);
        }
    };

    /**
     * fold_right a tuple of elements.
     *
     * Fold operations are sometimes called accumulate or reduce in other
     * languages or libraries.
     *
     * https://en.wikipedia.org/wiki/Fold_%28higher-order_function%29
     *
     * @param operation
     *      A callable that takes the current element being processed
     *      and the current state, and returns the state to be used
     *      to process the next element. Called for each element in
     *      the tuple.
     *
     * @return
     *      The final state of all of the operations.
     */
    template<
        typename... ElementTypes,
        typename InitType,
        typename CallableType>
    [[nodiscard]] CIB_CONSTEXPR inline static auto fold_right(
        std::tuple<ElementTypes...> const & elements,
        InitType const & initial_state,
        CallableType const & operation
    ) {
        return apply([&](auto const & ... element_pack){
            return (fold_helper{element_pack, operation} + ... + initial_state);
        }, elements);
    }

    /**
     * fold_right a tuple of elements.
     *
     * Fold operations are sometimes called accumulate or reduce in other
     * languages or libraries.
     *
     * https://en.wikipedia.org/wiki/Fold_%28higher-order_function%29
     *
     * @param operation
     *      A callable that takes the current element being processed
     *      and the current state, and returns the state to be used
     *      to process the next element. Called for each element in
     *      the tuple.
     *
     * @return
     *      The final state of all of the operations.
     */
    template<
        typename... ElementTypes,
        typename InitType,
        typename CallableType>
    [[nodiscard]] CIB_CONSTEXPR inline static auto fold_right(
        tuple<ElementTypes...> const & elements,
        InitType const & initial_state,
        CallableType const & operation
    ) {
        return apply([&](auto const & ... element_pack){
            return (fold_helper{element_pack, operation} + ... + initial_state);
        }, elements);
    }

    /**
     * Perform an operation on each element of an integral sequence.
     *
     * @param operation
     *      The operation to perform. Must be a callable that accepts a single parameter.
     */
    template<
        typename IntegralType,
        IntegralType... Indices,
        typename CallableType>
    CIB_CONSTEXPR inline static void for_each(
        [[maybe_unused]] std::integer_sequence<IntegralType, Indices...> const & sequence,
        CallableType const & operation
    ) {
        (operation(std::integral_constant<IntegralType, Indices>{}) , ...);
    }

    /**
     * Perform an operation on each element of an integral sequence from 0 to
     * num_elements.
     *
     * @param operation
     *      The operation to perform. Must be a callable that accepts a single parameter.
     */
    template<
        typename IntegralType,
        IntegralType NumElements,
        typename CallableType>
    CIB_CONSTEXPR inline void for_each(
        [[maybe_unused]] std::integral_constant<IntegralType, NumElements> const & num_elements,
        CallableType const & operation
    ) {
        CIB_CONSTEXPR auto seq = std::make_integer_sequence<IntegralType, NumElements>{};
        for_each(seq, operation);
    }

    /**
     * Perform an operation on each element of a tuple.
     *
     * @param operation
     *      The operation to perform. Must be a callable that accepts a single parameter.
     */
    template<
        typename... ElementTypes,
        typename CallableType>
    CIB_CONSTEXPR inline void for_each(
        std::tuple<ElementTypes...> const & elements,
        CallableType const & operation
    ) {
        apply([&](auto const & ... element_pack){
            (operation(element_pack) , ...);
        }, elements);
    }

    /**
     * Perform an operation on each element of a tuple.
     *
     * @param operation
     *      The operation to perform. Must be a callable that accepts a single parameter.
     */
    template<
        typename... ElementTypes,
        typename CallableType>
    CIB_CONSTEXPR inline void for_each(
        tuple<ElementTypes...> const & elements,
        CallableType const & operation
    ) {
        apply([&](auto const & ... element_pack){
            (operation(element_pack) , ...);
        }, elements);
    }
}


#endif //COMPILE_TIME_INIT_BUILD_META_HPP
