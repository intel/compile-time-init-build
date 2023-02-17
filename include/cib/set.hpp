#pragma once

#include <cib/detail/set_details.hpp>
#include <cib/tuple.hpp>

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace cib {
template <auto... Indexes, typename Operation>
[[nodiscard]] constexpr auto make_tuple_from_op(std::index_sequence<Indexes...>,
                                                Operation op) {
    return cib::make_tuple(
        op(std::integral_constant<std::size_t, Indexes>{})...);
}

template <typename MetaFunc, typename Tuple> struct create_demux_tags_t;

template <typename MetaFunc, typename... TupleElems>
struct create_demux_tags_t<MetaFunc, cib::tuple_impl<TupleElems...>> {
    constexpr static auto invoke() {
        auto type_names = cib::detail::create_type_names<
            MetaFunc, typename TupleElems::value_type...>(0);

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

        return type_names;
    }
};

/**
 * De-multiplex a tuple (tn) into a tuple of tuples grouped by a meta function
 * (meta_func).
 */
template <typename MetaFunc, typename Tuple>
[[nodiscard]] constexpr auto demux(Tuple t) {
// workaround for gcc bug
#if not(defined(__clang__)) && (defined(__GNUC__) || defined(__GNUG__))
#define tags create_demux_tags_t<MetaFunc, Tuple>::invoke()
#else
    constexpr auto tags = create_demux_tags_t<MetaFunc, Tuple>::invoke();
#endif

    constexpr std::size_t num_bins = tags.empty() ? 0 : (tags.back().src + 1);

    // FIXME: this should use generic algorithms
    // the number of elements in each bin in 'tags'
    constexpr std::array<std::size_t, num_bins> bin_size = [&]() {
        std::array<std::size_t, num_bins> bin_size_{};

        std::size_t begin = 0;
        std::size_t end = begin;
        for (std::size_t bin_index = 0; bin_index < num_bins; bin_index++) {
            while (end != tags.size() && tags[end].src == bin_index) {
                end++;
            }
            bin_size_[bin_index] = end - begin;
            begin = end;
        }

        return bin_size_;
    }();

    // FIXME: this should use generic algorithms
    // the offset of each bin in 'tags'
    constexpr std::array<std::size_t, num_bins> bin_offset = [&]() {
        std::array<std::size_t, num_bins> bin_offset_{};

        std::size_t begin = 0;
        std::size_t end = begin;
        for (std::size_t bin_index = 0; bin_index < num_bins; bin_index++) {
            while (end != tags.size() && tags[end].src == bin_index) {
                end++;
            }
            bin_offset_[bin_index] = begin;
            begin = end;
        }

        return bin_offset_;
    }();

    return cib::make_tuple_from_op(
        std::make_index_sequence<num_bins>{}, [&](auto bin_index) {
            return cib::make_tuple_from_op(
                std::make_index_sequence<bin_size[bin_index]>{},
                [&](auto offset_into_bin) {
                    constexpr auto tuple_index =
                        tags[bin_offset[bin_index] + offset_into_bin].index;
                    return t.get(index_<tuple_index>);
                });
        });

#undef tags
}
} // namespace cib
