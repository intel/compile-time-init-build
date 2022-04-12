#include "type_pack_element.hpp"

#include <type_traits>
#include <utility>
#include <array>


#ifndef COMPILE_TIME_INIT_BUILD_TUPLE_HPP
#define COMPILE_TIME_INIT_BUILD_TUPLE_HPP


namespace cib::detail {
    template<typename T>
    struct tuple_element {
        T value;
    };

    template<typename... Tn>
    struct ordered_set : public tuple_element<Tn>... {
        constexpr ordered_set(Tn... values)
            : tuple_element<Tn>{values}...
        {}

        template<int Index>
        constexpr auto const & get() const {
            using T = type_pack_element<Index, Tn...>;
            return tuple_element<T>::value;
        }

        template<typename T>
        constexpr T const & get() const {
            return tuple_element<T>::value;
        }

        constexpr static auto size() {
            return sizeof...(Tn);
        }

        CIB_CONSTEXPR bool operator==(ordered_set<Tn...> const & rhs) const {
            return ((tuple_element<Tn>::value == rhs.tuple_element<Tn>::value) && ... && true);
        }

        template<typename... RhsTn>
        CIB_CONSTEXPR bool operator==(ordered_set<RhsTn...>) const {
            return false;
        }

        template<typename... RhsTn>
        CIB_CONSTEXPR bool operator!=(ordered_set<RhsTn...> const & rhs) const {
            return !(*this == rhs);
        }
    };

    template<typename Callable, typename... Values>
    constexpr auto apply(Callable operation, ordered_set<Values...> const & t) {
        return operation(t.tuple_element<Values>::value...);
    }

    struct index_pair {
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
            std::array<index_pair, total_num_elements> indices{};
            unsigned int flat_index = 0;
            for (unsigned int outer_index = 0; outer_index < num_tuples; outer_index++) {
                for (unsigned int inner_index = 0; inner_index < tuple_sizes[outer_index]; inner_index++) {
                    indices[flat_index] = {outer_index, inner_index};
                    flat_index += 1;
                }
            }
            return indices;
        }();

        auto const outer_tuple = ordered_set{tuples...};
        return ordered_set{outer_tuple.template get<element_indices[Indices].outer>().template get<element_indices[Indices].inner>()...};
    }

    template<typename... Tuples>
    constexpr auto tuple_cat(Tuples... tuples) {
        constexpr auto total_num_elements = (tuples.size() + ...);
        return tuple_cat_impl(std::make_integer_sequence<int, total_num_elements>{}, tuples...);
    }
}

namespace std {
    template<typename... Values>
    struct tuple_size<cib::detail::ordered_set<Values...>>
        : std::integral_constant<std::size_t, sizeof...(Values)>
    {};
}


#endif //COMPILE_TIME_INIT_BUILD_TUPLE_HPP
