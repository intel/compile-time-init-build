#pragma once


#include <sc/fwd.hpp>

#include <tuple>
#include <utility>
#include <type_traits>


namespace sc::detail {
    template<
        typename ValueT,
        typename CallableT>
    struct fold_helper {
        ValueT value;
        CallableT op;

        constexpr fold_helper(
            ValueT value,
            CallableT op
        )
            : value{value}
            , op{op}
        {}

        template<typename StateT>
        [[nodiscard]] constexpr auto operator+(
            StateT state
        ) const {
            return op(value, state);
        }
    };


    template<
        typename... T,
        typename InitT,
        typename CallableT>
    [[nodiscard]] constexpr static auto fold_right(
        std::tuple<T...> t,
        InitT init,
        CallableT op
    ) {
        return std::apply([&](auto... e){
            return (fold_helper{e, op} + ... + init);
        }, t);
    }


    template<
        typename T,
        T... i,
        typename InitT,
        typename CallableT>
    [[nodiscard]] constexpr static auto fold_right(
        std::integer_sequence<T, i...>,
        InitT init,
        CallableT op
    ) {
        return (fold_helper{int_<i>, op} + ... + init);
    }


    template<
        typename T,
        T size,
        typename InitT,
        typename CallableT>
    [[nodiscard]] constexpr auto fold_right(
        std::integral_constant<T, size>,
        InitT init,
        CallableT op
    ) {
        constexpr auto seq = std::make_integer_sequence<T, size>{};
        return fold_right(seq, init, op);
    }


    template<
        typename T,
        T... i,
        typename CallableT>
    [[nodiscard]] constexpr static auto transform(
        std::integer_sequence<T, i...>,
        CallableT op
    ) {
        return std::make_tuple(op(int_<i>)...);
    }


    template<
        typename T,
        T size,
        typename CallableT>
    [[nodiscard]] constexpr auto transform(
        std::integral_constant<T, size>,
        CallableT op
    ) {
        constexpr auto seq = std::make_integer_sequence<T, size>{};
        return transform(seq, op);
    }
}