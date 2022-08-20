#include <type_traits>
#include <utility>
#include <array>

#include <cib/detail/compiler.hpp>

#ifndef COMPILE_TIME_INIT_BUILD_TUPLE_HPP
#define COMPILE_TIME_INIT_BUILD_TUPLE_HPP


namespace cib {
    template<typename T>
    struct tag_t {};

    template<typename T>
    constexpr static tag_t<T> tag_{};

    template<int Index>
    struct index_t {};

    template<int Index>
    constexpr static index_t<Index> index_{};

    template<typename T>
    struct index_metafunc_t {};

    template<typename T>
    constexpr static index_metafunc_t<T> index_metafunc_{};


    struct self_type {
        template<typename T>
        using invoke = T;
    } ;

    constexpr static index_metafunc_t<self_type> self_type_index{};


    template<typename T, int Index, typename... IndexTs>
    struct tuple_element;

    template<typename T, int Index>
    struct tuple_element<T, Index> {
        using value_type = T;
        T value;

        [[nodiscard]] constexpr T const & get(index_t<Index>) const {
            return value;
        }
    };

    template<typename T, int Index, typename IndexT0>
    struct tuple_element<T, Index, IndexT0> {
        using value_type = T;
        T value;

        [[nodiscard]] constexpr T const & get(index_t<Index>) const {
            return value;
        }

        [[nodiscard]] constexpr T const & get(tag_t<IndexT0>) const {
            return value;
        }
    };

    template<typename T, int Index, typename IndexT0, typename IndexT1>
    struct tuple_element<T, Index, IndexT0, IndexT1> {
        using value_type = T;
        T value;

        [[nodiscard]] constexpr T const & get(index_t<Index>) const {
            return value;
        }

        [[nodiscard]] constexpr T const & get(tag_t<IndexT0>) const {
            return value;
        }

        [[nodiscard]] constexpr T const & get(tag_t<IndexT1>) const {
            return value;
        }
    };

    template<typename T, int Index, typename IndexT0, typename IndexT1, typename IndexT2>
    struct tuple_element<T, Index, IndexT0, IndexT1, IndexT2> {
        using value_type = T;
        T value;

        [[nodiscard]] constexpr T const & get(index_t<Index>) const {
            return value;
        }

        [[nodiscard]] constexpr T const & get(tag_t<IndexT0>) const {
            return value;
        }

        [[nodiscard]] constexpr T const & get(tag_t<IndexT1>) const {
            return value;
        }

        [[nodiscard]] constexpr T const & get(tag_t<IndexT2>) const {
            return value;
        }
    };

    namespace detail {
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
    }

    template<typename... TupleElementTs>
    struct tuple_impl : public TupleElementTs... {
        CIB_CONSTEXPR tuple_impl(TupleElementTs... values)
            : TupleElementTs{values}...
        {}

        template <std::enable_if_t<(sizeof...(TupleElementTs) >= 0), bool> = true>
        CIB_CONSTEXPR tuple_impl()
            : TupleElementTs{}...
        {}

        using TupleElementTs::get...;

        [[nodiscard]] CIB_CONSTEXPR static int size() {
            return sizeof...(TupleElementTs);
        }

        template<typename Callable>
        CIB_CONSTEXPR auto apply(Callable operation) const {
            return operation(TupleElementTs::value...);
        }

        /**
         * Perform an operation on each element.
         *
         * @param operation
         *      The operation to perform. Must be a callable that accepts a single parameter.
         */
        template<typename Callable>
        CIB_CONSTEXPR void for_each(Callable operation) const {
            (operation(TupleElementTs::value) , ...);
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
            typename InitType,
            typename CallableType>
        [[nodiscard]] CIB_CONSTEXPR inline auto fold_right(
            InitType const & initial_state,
            CallableType const & operation
        ) const {
            return (detail::fold_helper{TupleElementTs::value, operation} + ... + initial_state);
        }

        [[nodiscard]] CIB_CONSTEXPR bool operator==(tuple_impl<TupleElementTs...> const & rhs) const {
            return ((this->TupleElementTs::value == rhs.TupleElementTs::value) && ... && true);
        }

        [[nodiscard]] CIB_CONSTEXPR bool operator!=(tuple_impl<TupleElementTs...> const & rhs) const {
            return ((this->TupleElementTs::value != rhs.TupleElementTs::value) || ... || false);
        }
    };

    template<int... Indices, typename... Ts>
    [[nodiscard]] constexpr auto make_tuple_impl(std::integer_sequence<int, Indices...>, Ts const & ... values) {
        return tuple_impl{tuple_element<Ts, Indices>{values}...};
    }

    template<int... Indices, typename IndexMetafuncT0, typename... Ts>
    [[nodiscard]] constexpr auto make_tuple_impl(std::integer_sequence<int, Indices...>, index_metafunc_t<IndexMetafuncT0>, Ts const & ... values) {
        return tuple_impl{tuple_element<Ts, Indices, typename IndexMetafuncT0::template invoke<Ts>>{values}...};
    }

    template<int... Indices, typename IndexMetafuncT0, typename IndexMetafuncT1, typename... Ts>
    [[nodiscard]] constexpr auto make_tuple_impl(
        std::integer_sequence<int, Indices...>,
        index_metafunc_t<IndexMetafuncT0>,
        index_metafunc_t<IndexMetafuncT1>,
        Ts const & ... values
    ) {
        return tuple_impl{
            tuple_element<Ts, Indices,
                typename IndexMetafuncT0::template invoke<Ts>,
                typename IndexMetafuncT1::template invoke<Ts>
            >{values}...
        };
    }

    template<int... Indices, typename IndexMetafuncT0, typename IndexMetafuncT1, typename IndexMetafuncT2, typename... Ts>
    [[nodiscard]] constexpr auto make_tuple_impl(
        std::integer_sequence<int, Indices...>,
        index_metafunc_t<IndexMetafuncT0>,
        index_metafunc_t<IndexMetafuncT1>,
        index_metafunc_t<IndexMetafuncT2>,
        Ts const & ... values
    ) {
        return tuple_impl{
            tuple_element<Ts, Indices,
                typename IndexMetafuncT0::template invoke<Ts>,
                typename IndexMetafuncT1::template invoke<Ts>,
                typename IndexMetafuncT2::template invoke<Ts>
            >{values}...
        };
    }

    template<typename... Ts>
    [[nodiscard]] constexpr auto make_tuple(Ts const & ... values) {
        return make_tuple_impl(std::make_integer_sequence<int, sizeof...(values)>{}, values...);
    }

    template<typename IndexMetafuncT0, typename... Ts>
    [[nodiscard]] constexpr auto make_tuple(index_metafunc_t<IndexMetafuncT0>, Ts const & ... values) {
        return make_tuple_impl(std::make_integer_sequence<int, sizeof...(values)>{}, index_metafunc_<IndexMetafuncT0>, values...);
    }

    template<typename IndexMetafuncT0, typename IndexMetafuncT1, typename... Ts>
    [[nodiscard]] constexpr auto make_tuple(
        index_metafunc_t<IndexMetafuncT0>,
        index_metafunc_t<IndexMetafuncT1>,
        Ts const & ... values
    ) {
        return make_tuple_impl(
            std::make_integer_sequence<int, sizeof...(values)>{},
            index_metafunc_<IndexMetafuncT0>,
            index_metafunc_<IndexMetafuncT1>,
            values...
        );
    }

    template<typename IndexMetafuncT0, typename IndexMetafuncT1, typename IndexMetafuncT2, typename... Ts>
    [[nodiscard]] constexpr auto make_tuple(
        index_metafunc_t<IndexMetafuncT0>,
        index_metafunc_t<IndexMetafuncT1>,
        index_metafunc_t<IndexMetafuncT2>,
        Ts const & ... values
    ) {
        return make_tuple_impl(
            std::make_integer_sequence<int, sizeof...(values)>{},
            index_metafunc_<IndexMetafuncT0>,
            index_metafunc_<IndexMetafuncT1>,
            index_metafunc_<IndexMetafuncT2>,
            values...
        );
    }

    template<typename... Ts>
    using tuple = decltype(cib::make_tuple(std::declval<Ts>()...));

    template<typename Callable, typename... Elements>
    CIB_CONSTEXPR auto apply(Callable operation, tuple_impl<Elements...> const & t) {
        return t.apply(operation);
    }

    template<
        typename MetaFunc,
        typename Tuple,
        typename Operation>
    [[nodiscard]] CIB_CONSTEXPR auto transform(
        MetaFunc meta_func,
        Tuple tuple,
        Operation op
    ) {
        return tuple.apply([&](auto... elements){
            return cib::make_tuple(meta_func, op(elements)...);
        });
    }

    struct tuple_pair {
        unsigned int outer;
        unsigned int inner;
    };

    template<
        int... Indices,
        typename... Tuples>
    constexpr auto tuple_cat_impl(
        std::integer_sequence<int, Indices...>,
        Tuples... tuples
    ) {
        constexpr auto num_tuples = sizeof...(tuples);
        constexpr std::array<unsigned int, num_tuples> tuple_sizes{tuples.size()...};
        constexpr auto element_indices = [&](){
            constexpr auto total_num_elements = (tuples.size() + ...);
            std::array<tuple_pair, total_num_elements> indices{};
            unsigned int flat_index = 0;
            for (unsigned int outer_index = 0; outer_index < num_tuples; outer_index++) {
                for (unsigned int inner_index = 0; inner_index < tuple_sizes[outer_index]; inner_index++) {
                    indices[flat_index] = {outer_index, inner_index};
                    flat_index += 1;
                }
            }
            return indices;
        }();

        auto const outer_tuple = cib::make_tuple(tuples...);

        return cib::make_tuple(
            outer_tuple.get(index_<element_indices[Indices].outer>).get(index_<element_indices[Indices].inner>)...
        );
    }

    template<typename... Tuples>
    constexpr auto tuple_cat(Tuples... tuples) {
        constexpr auto total_num_elements = (tuples.size() + ...);
        return tuple_cat_impl(std::make_integer_sequence<int, total_num_elements>{}, tuples...);
    }
}

namespace std {
    template<typename... Elements>
    struct tuple_size<cib::tuple_impl<Elements...>>
        : std::integral_constant<std::size_t, sizeof...(Elements)>
    {};
}


#endif //COMPILE_TIME_INIT_BUILD_TUPLE_HPP
