#pragma once
#include <lookup/entry.hpp>
#include <lookup/input.hpp>

#include <stdx/bitset.hpp>
#include <stdx/compiler.hpp>
#include <stdx/cx_map.hpp>
#include <stdx/cx_vector.hpp>
#include <stdx/tuple.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <numeric>
#include <span>

namespace msg::detail {

template <std::size_t N> struct smallest_storage {
    // select a minimum sized type for indexing into the RLE data blob
    static CONSTEVAL auto select_index_storage() {
        if constexpr (N <= std::numeric_limits<std::uint8_t>::max()) {
            return std::uint8_t{};
        } else if constexpr (N <= std::numeric_limits<std::uint16_t>::max()) {
            return std::uint16_t{};
        } else if constexpr (N <= std::numeric_limits<std::uint32_t>::max()) {
            return std::uint32_t{};
        } else {
            return std::size_t{};
        }
    }

    using type = decltype(select_index_storage());
};

template <std::size_t N>
using smallest_storage_type = typename smallest_storage<N>::type;

template <typename BitSetType> struct rle_codec {
    using bitset_type = BitSetType;
    constexpr static auto num_bits = BitSetType::size();

    // assume worst case of each bitmap being alternating bit values
    using max_rle_data_type = stdx::cx_vector<std::byte, num_bits * 2>;

    constexpr static auto encode(bitset_type const &bitset)
        -> max_rle_data_type {
        max_rle_data_type data{};
        std::size_t count{0};
        bool last{false};
        for (std::size_t bit{0}; bit < num_bits; ++bit) {
            if (bitset[bit] != last) {
                data.push_back(static_cast<std::byte>(count));
                last = !last;
                count = 1;
            } else {
                if (++count > 255) {
                    // we have overflowed the max byte. we need to
                    // reset our count and indicate there are no alternate bits
                    data.push_back(std::byte{255});
                    data.push_back(std::byte{0});
                    count = 1;
                }
            }
        }
        // final byte
        data.push_back(static_cast<std::byte>(count));
        return data;
    }

    constexpr static auto decode(std::byte const *rle_data) -> bitset_type {
        bitset_type result{};

        std::size_t bit{0};
        bool bit_val{false};

        // accumulate the correct total number of bits
        while (bit < num_bits) {
            // get the next number of consecutive bits
            auto cur_bits = static_cast<std::size_t>(*rle_data++);
            if (bit_val) {
                // write cur_bits of 1s
                while (cur_bits-- > 0) {
                    result.set(bit++);
                }
            } else {
                // skip cur_bits of 0s
                bit += cur_bits;
            }
            bit_val = !bit_val;
        }

        return result;
    }
};

} // namespace msg::detail
