#ifndef COMPILE_TIME_INIT_BUILD_META_HPP
#define COMPILE_TIME_INIT_BUILD_META_HPP


#include "compiler.hpp"

#include <tuple>
#include <type_traits>


namespace cib::detail {
    template<int value>
    CIB_CONSTEXPR auto int_ = std::integral_constant<int, value>{};

    template<
        typename ValueT,
        typename CallableT>
    struct fold_helper {
        ValueT const & value;
        CallableT const & op;

        CIB_CONSTEXPR fold_helper(
            ValueT const & value,
            CallableT const & op
        )
            : value{value}
            , op{op}
        {}

        template<typename StateT>
        [[nodiscard]] CIB_CONSTEXPR inline auto operator+(
            StateT const & state
        ) const {
            return op(value, state);
        }
    };

    template<
        typename... T,
        typename InitT,
        typename CallableT>
    [[nodiscard]] CIB_CONSTEXPR inline static auto fold_right(
        std::tuple<T...> const & t,
        InitT const & init,
        CallableT const & op
    ) {
        return std::apply([&](auto const & ... e){
            return (fold_helper{e, op} + ... + init);
        }, t);
    }

    template<
        typename T,
        T... i,
        typename CallableT>
    CIB_CONSTEXPR inline static void for_each(
        std::integer_sequence<T, i...> const &,
        CallableT const & op
    ) {
        (op(std::integral_constant<int, i>{}) , ...);
    }

    template<
        typename T,
        T size,
        typename CallableT>
    CIB_CONSTEXPR inline void for_each(
        std::integral_constant<T, size> const &,
        CallableT const & op
    ) {
        CIB_CONSTEXPR auto seq = std::make_integer_sequence<T, size>{};
        for_each(seq, op);
    }

    template<
        typename... Ts,
        typename CallableT>
    CIB_CONSTEXPR inline void for_each(
        std::tuple<Ts...> const & tuple,
        CallableT const & op
    ) {
        std::apply([&](auto const & ... pack){
            (op(pack) , ...);
        }, tuple);
    }
}


#endif //COMPILE_TIME_INIT_BUILD_META_HPP
