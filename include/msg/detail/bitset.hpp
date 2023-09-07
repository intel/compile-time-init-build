#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstdint>
#include <numeric>

namespace msg::detail {

// NOTE: std has a bitset, however std::bitset does not have a way
//       to efficiently iterate over its set bits; see `for_each` below.
template <std::size_t NumBits> struct bitset {
    constexpr static std::size_t size = (NumBits + 31) / 32;
    std::array<uint32_t, size> data{};

    constexpr bitset() = default;
    constexpr explicit bitset(std::integral auto... args) {
        (add(static_cast<std::size_t>(args)), ...);
    }

    template <auto RhsNumBits>
    constexpr inline auto operator==(bitset<RhsNumBits> const &rhs) const
        -> bool {
        constexpr auto rhs_size = bitset<RhsNumBits>::size;
        constexpr auto min_size = std::min(size, rhs_size);

        bool match = true;
        for (auto i = std::size_t{}; i < min_size; i++) {
            match &= (data[i] == rhs.data[i]);
        }

        if constexpr (size > rhs_size) {
            for (auto i = min_size; i < size; i++) {
                match &= (data[i] == 0);
            }
        } else if constexpr (size < rhs_size) {
            for (auto i = min_size; i < rhs_size; i++) {
                match &= (rhs.data[i] == 0);
            }
        }

        return match;
    }

    // template <std::size_t RhsNumBits>
    // constexpr inline bool operator!=(bitset<RhsNumBits> const &rhs) const {
    //     return !(*this == rhs);
    // }

    constexpr inline auto add(std::size_t i) -> void {
        data[i / 32] |= (1u << (i % 32));
    }

    constexpr inline auto remove(std::size_t i) -> void {
        data[i / 32] &= ~(1u << (i % 32));
    }

    [[nodiscard]] constexpr inline auto contains(std::size_t i) const -> bool {
        return data[i / 32] & (1u << (i % 32));
    }

    template <typename T>
    constexpr auto for_each(T const &action) const -> void {
        for (auto i = std::size_t{}; i < size; i++) {
            auto const bit_index_base = i * 32;

            auto current_bits = data[i];
            while (current_bits != 0u) {
                auto const bit_index_offset = std::countr_zero(current_bits);
                auto const bit_index =
                    bit_index_base + static_cast<std::size_t>(bit_index_offset);
                current_bits &= ~(1u << bit_index_offset);
                action(bit_index);
            }
        }
    }

    constexpr inline auto operator&=(bitset const &rhs) -> bitset & {
        for (auto i = std::size_t{}; i < size; i++) {
            data[i] &= rhs.data[i];
        }

        return *this;
    }

    constexpr inline auto operator&(bitset const &rhs) const -> bitset {
        auto result = *this;
        result &= rhs;
        return result;
    }

    constexpr inline auto operator|=(bitset const &rhs) -> bitset & {
        for (auto i = std::size_t{}; i < size; i++) {
            data[i] |= rhs.data[i];
        }

        return *this;
    }

    constexpr inline auto operator|(bitset const &rhs) const -> bitset {
        auto result = *this;
        result |= rhs;
        return result;
    }

    [[nodiscard]] constexpr inline auto empty() const -> bool {
        // NOTE: bitwise-OR all values together to avoid logical-OR
        // short-circuit branching.
        uint32_t const merged = std::accumulate(data.begin(), data.end(),
                                                uint32_t{}, std::bit_or{});

        return merged == 0;
    }

    [[nodiscard]] constexpr inline auto count() const -> int {
        return std::accumulate(
            data.begin(), data.end(), 0,
            [](int val, uint32_t elem) { return val + std::popcount(elem); });
    }
};

} // namespace msg::detail
