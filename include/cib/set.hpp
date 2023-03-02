#pragma once

#include <cib/detail/set_details.hpp>
#include <cib/tuple.hpp>

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace cib {
namespace detail {
template <auto... Indexes, typename Operation>
[[nodiscard]] constexpr auto make_tuple_from_op(std::index_sequence<Indexes...>,
                                                [[maybe_unused]] Operation op) {
    return cib::make_tuple(
        op(std::integral_constant<std::size_t, Indexes>{})...);
}

template <template <typename> typename MetaFunc, typename... TupleElems>
[[nodiscard]] constexpr auto
create_demux_tags(cib::tuple<TupleElems...> const &) {
    auto type_names = create_type_names<MetaFunc, TupleElems...>(0);

    if constexpr (sizeof...(TupleElems) > 0) {
        // assign all type_names with the same name the same src id
        auto prev_name = type_names.front();
        std::size_t name_dst_index = 0;
        for (auto &name : type_names) {
            if (name != prev_name) {
                prev_name = name;
                ++name_dst_index;
            }
            name.src = name_dst_index;
        }
    }

    return type_names;
}

struct bin {
    std::size_t offset{};
    std::size_t size{};
};

template <std::size_t N>
[[nodiscard]] constexpr auto create_bins(const auto &tags) {
    std::array<bin, N> bins{};
    auto index = std::size_t{};
    auto offset = std::size_t{};
    for (auto &tag : tags) {
        if (tag.src != index) {
            bins[++index].offset = offset;
        }
        ++bins[index].size;
        ++offset;
    }
    return bins;
}
} // namespace detail

/**
 * De-multiplex a tuple (tn) into a tuple of tuples grouped by a meta function
 * (meta_func).
 */
template <template <typename> typename MetaFunc, typename Tuple>
[[nodiscard]] constexpr auto demux(Tuple t) {
    constexpr auto tags = detail::create_demux_tags<MetaFunc>(t);
    constexpr std::size_t num_bins = tags.empty() ? 0 : (tags.back().src + 1);
    constexpr auto bins = detail::create_bins<num_bins>(tags);

    return detail::make_tuple_from_op(
        std::make_index_sequence<std::size(bins)>{}, [&](auto bin_index) {
            return detail::make_tuple_from_op(
                std::make_index_sequence<bins[bin_index].size>{},
                [&](auto offset_into_bin) {
                    constexpr auto tuple_index =
                        tags[bins[bin_index].offset + offset_into_bin].index;
                    return t[index<tuple_index>];
                });
        });
}
} // namespace cib
