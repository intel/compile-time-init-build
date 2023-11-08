#pragma once

#include <msg/detail/indexed_handler_common.hpp>
#include <msg/detail/rle_codec.hpp>

#include <stdx/compiler.hpp>

namespace msg {

template <typename RleStorageT, typename... IndicesT>
struct rle_indices : IndicesT... {
    CONSTEVAL explicit rle_indices(RleStorageT data, IndicesT... index_args)
        : IndicesT{index_args}..., storage{data} {}

    constexpr auto operator()(auto const &data) const {
        // proxy to allow intersection without materializing a full bitset.
        return detail::rle_intersect{
            storage.decode(this->IndicesT::operator()(data))...};
    }

    // index entries will map into this storage to decode RLE data
    RleStorageT storage;
};

} // namespace msg
