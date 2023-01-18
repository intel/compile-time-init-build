#pragma once

#include <cib/detail/type_pack_element.hpp>

#include <array>
#include <utility>

namespace cib::detail {
template <typename... Values> struct type_list {
    constexpr static auto size = sizeof...(Values);

    template <unsigned int Index> constexpr auto get() const {
        constexpr auto value = type_pack_element<Index, Values...>{};
        return value;
    }
};

struct type_list_index_pair {
    unsigned int outer;
    unsigned int inner;
};

template <unsigned int... Indices, typename... TypeLists>
constexpr auto
type_list_cat_impl(std::integer_sequence<unsigned int, Indices...>,
                   TypeLists... type_lists) {
    constexpr auto num_type_lists = sizeof...(type_lists);
    constexpr std::array<unsigned int, num_type_lists> type_list_sizes{
        type_lists.size...};

    // gcc thinks this is unused
    [[maybe_unused]] constexpr auto element_indices = [&]() {
        constexpr auto total_num_elements = (type_lists.size + ... + 0);
        std::array<cib::detail::type_list_index_pair, total_num_elements>
            indices{};
        unsigned int flat_index = 0;
        for (unsigned int outer_index = 0; outer_index < num_type_lists;
             outer_index++) {
            for (unsigned int inner_index = 0;
                 inner_index < type_list_sizes[outer_index]; inner_index++) {
                indices[flat_index] = {outer_index, inner_index};
                flat_index += 1;
            }
        }
        return indices;
    }();

    auto const outer_type_list = type_list<decltype(type_lists)...>{};

    return type_list<
        decltype(outer_type_list.template get<element_indices[Indices].outer>()
                     .template get<element_indices[Indices].inner>())...>{};
}

template <typename... TypeLists>
constexpr auto type_list_cat(TypeLists... type_lists) {
    constexpr auto num_type_lists_with_elements = (type_lists.size + ... + 0);

    return type_list_cat_impl(
        std::make_integer_sequence<unsigned int,
                                   num_type_lists_with_elements>{},
        type_lists...);
}
} // namespace cib::detail
