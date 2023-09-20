#pragma once

#include <msg/detail/indexed_handler_common.hpp>

#include <stdx/compiler.hpp>

namespace msg {

template <typename RleStorageT, typename... IndicesT>
struct rle_indices : IndicesT... {
    CONSTEVAL explicit rle_indices(RleStorageT data, IndicesT... index_args)
        : IndicesT{index_args}..., storage{data} {}

    constexpr auto operator()(auto const &data) const {
        // TODO: efficient bitand that doesn't need to materialise full bitset

        // use the index to decode the bitset from storage
        return (storage.get(this->IndicesT::operator()(data)) & ...);
    }

    // index entries will map into this storage to decode RLE data
    RleStorageT storage;
};

} // namespace msg
