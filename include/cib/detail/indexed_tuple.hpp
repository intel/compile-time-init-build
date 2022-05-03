#include <type_traits>
#include <utility>
#include <array>


#ifndef COMPILE_TIME_INIT_BUILD_INDEXED_TUPLE_HPP
#define COMPILE_TIME_INIT_BUILD_INDEXED_TUPLE_HPP


namespace cib::detail {
    template<typename T>
    struct tag_ {};

    template<int Index>
    struct index_ {};

    template<typename T>
    struct index_metafunc_ {};

    template<typename T, int Index, typename... IndexTs>
    struct indexed_tuple_element;

    template<typename T, int Index>
    struct indexed_tuple_element<T, Index> {
        T value;

        [[nodiscard]] constexpr T const & get(index_<Index>) const {
            return value;
        }
    };

    template<typename T, int Index, typename IndexT0>
    struct indexed_tuple_element<T, Index, IndexT0> {
        T value;

        [[nodiscard]] constexpr T const & get(index_<Index>) const {
            return value;
        }

        [[nodiscard]] constexpr T const & get(tag_<IndexT0>) const {
            return value;
        }
    };

    template<typename T, int Index, typename IndexT0, typename IndexT1>
    struct indexed_tuple_element<T, Index, IndexT0, IndexT1> {
        T value;

        [[nodiscard]] constexpr T const & get(index_<Index>) const {
            return value;
        }

        [[nodiscard]] constexpr T const & get(tag_<IndexT0>) const {
            return value;
        }

        [[nodiscard]] constexpr T const & get(tag_<IndexT1>) const {
            return value;
        }
    };

    template<typename... IndexedTupleElementTs>
    struct indexed_tuple : public IndexedTupleElementTs... {
        constexpr indexed_tuple(IndexedTupleElementTs... values)
            : IndexedTupleElementTs{values}...
        {}

        using IndexedTupleElementTs::get...;

        [[nodiscard]] constexpr int size() const {
            return sizeof...(IndexedTupleElementTs);
        }

        template<typename Callable>
        constexpr auto apply(Callable operation) const {
            return operation(IndexedTupleElementTs::value...);
        }
    };

    template<int... Indices, typename... Ts>
    [[nodiscard]] constexpr auto make_indexed_tuple(std::integer_sequence<int, Indices...>, Ts const & ... values) {
        return indexed_tuple{indexed_tuple_element<Ts, Indices>{values}...};
    }

    template<int... Indices, typename IndexMetafuncT0, typename... Ts>
    [[nodiscard]] constexpr auto make_indexed_tuple(std::integer_sequence<int, Indices...>, index_metafunc_<IndexMetafuncT0>, Ts const & ... values) {
        return indexed_tuple{indexed_tuple_element<Ts, Indices, typename IndexMetafuncT0::template invoke<Ts>>{values}...};
    }

    template<typename... Ts>
    [[nodiscard]] constexpr auto make_indexed_tuple(Ts const & ... values) {
        return make_indexed_tuple(std::make_integer_sequence<int, sizeof...(values)>{}, values...);
    }

    template<typename IndexMetafuncT0, typename... Ts>
    [[nodiscard]] constexpr auto make_indexed_tuple(index_metafunc_<IndexMetafuncT0>, Ts const & ... values) {
        return make_indexed_tuple(std::make_integer_sequence<int, sizeof...(values)>{}, index_metafunc_<IndexMetafuncT0>{}, values...);
    }


    template<typename Callable, typename... Elements>
    constexpr auto apply(Callable operation, indexed_tuple<Elements...> const & t) {
        return t.apply(operation);
    }

    struct indexed_tuple_pair {
        unsigned int outer;
        unsigned int inner;
    };

    template<
        int... Indices,
        typename... Tuples>
    constexpr auto indexed_tuple_cat_impl(
        std::integer_sequence<int, Indices...>,
        Tuples... tuples
    ) {
        constexpr auto num_tuples = sizeof...(tuples);
        constexpr std::array<unsigned int, num_tuples> tuple_sizes{tuples.size()...};
        constexpr auto element_indices = [&](){
            constexpr auto total_num_elements = (tuples.size() + ...);
            std::array<indexed_tuple_pair, total_num_elements> indices{};
            unsigned int flat_index = 0;
            for (unsigned int outer_index = 0; outer_index < num_tuples; outer_index++) {
                for (unsigned int inner_index = 0; inner_index < tuple_sizes[outer_index]; inner_index++) {
                    indices[flat_index] = {outer_index, inner_index};
                    flat_index += 1;
                }
            }
            return indices;
        }();

        auto const outer_tuple = make_indexed_tuple(tuples...);

        return make_indexed_tuple(
            outer_tuple.get(index_<element_indices[Indices].outer>{}).get(index_<element_indices[Indices].inner>{})...
        );
    }

    template<typename... Tuples>
    constexpr auto indexed_tuple_cat(Tuples... tuples) {
        constexpr auto total_num_elements = (tuples.size() + ...);
        return indexed_tuple_cat_impl(std::make_integer_sequence<int, total_num_elements>{}, tuples...);
    }
}

namespace std {
    template<typename... Values>
    struct tuple_size<cib::detail::indexed_tuple<Values...>>
        : std::integral_constant<std::size_t, sizeof...(Values)>
    {};
}


#endif //COMPILE_TIME_INIT_BUILD_INDEXED_TUPLE_HPP
