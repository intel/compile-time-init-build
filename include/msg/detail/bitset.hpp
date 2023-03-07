#pragma once

#include <array>
#include <cstdint>
#include <bit>
#include <concepts>
#include <numeric>
#include <algorithm>

namespace msg::detail {

// NOTE: std has a bitset, however std::bitset does not have a way
//       to efficiently iterate over its set bits; see `for_each` below.
template<std::size_t NumBits>
struct bitset {
    static constexpr int size = (NumBits + 31) / 32;
    std::array<uint32_t, size> data{};


    constexpr bitset(std::integral auto... args) {
        (add(static_cast<std::size_t>(args)), ...);
    }

    template<auto RhsNumBits>
    constexpr inline bool operator==(bitset<RhsNumBits> const & rhs) const {
        auto constexpr min_size = std::min(size, rhs.size);

        bool match = true;
        for (auto i = 0; i < min_size; i++) {
            match &= (data[i] == rhs.data[i]);
        }

        if constexpr (size > rhs.size) {
            for (auto i = min_size; i < size; i++) {
                match &= (data[i] == 0);
            }
        } else if constexpr (size < rhs.size) {
            for (auto i = min_size; i < rhs.size; i++) {
                match &= (rhs.data[i] == 0);
            }
        }

        return match;
    }

    template<std::size_t RhsNumBits>
    constexpr inline bool operator!=(bitset<RhsNumBits> const & rhs) const {
        return !(*this == rhs);
    }

    constexpr inline void add(std::size_t i) {
        data[i / 32] |= (1 << (i % 32));
    }

    constexpr inline void remove(std::size_t i) {
        data[i / 32] &= ~(1 << (i % 32));
    }

    [[nodiscard]] constexpr inline bool contains(std::size_t i) const {
        return data[i / 32] & (1 << (i % 32));
    }

    template<typename T>
    constexpr void for_each(T const & action) const {
        for (auto i = std::size_t{}; i < size; i++) {
            auto const bit_index_base = i * 32;

            auto current_bits = data[i];
            while (current_bits != 0u) {
                auto const bit_index_offset = std::countr_zero(current_bits);
                auto const bit_index = bit_index_base + static_cast<std::size_t>(bit_index_offset);
                current_bits &= ~(1 << bit_index_offset);
                action(bit_index);
            }
        }
    }

    constexpr inline bitset & operator&=(bitset const & rhs) {
        for (auto i = std::size_t{}; i < size; i++) {
            data[i] &= rhs.data[i];
        }

        return *this;
    }

    constexpr inline bitset operator&(bitset const & rhs) const {
        auto result = *this;
        result &= rhs;
        return result;
    }

    [[nodiscard]] constexpr inline bool empty() const {
        // NOTE: bitwise-OR all values together to avoid logical-OR
        // short-circuit branching.
        uint32_t const merged =
            std::accumulate(data.begin(), data.end(), uint32_t{}, std::bit_or<uint32_t>());

        return merged == 0;
    }

    [[nodiscard]] constexpr inline int count() const {
        return std::accumulate(data.begin(), data.end(), 0, [](int val, uint32_t elem){
            return val + std::popcount(elem);
        });
    }
};

}
