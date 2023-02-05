#pragma once

#include <array>
#include <cstdint>

namespace msg::detail {

template<int NumBits>
struct bitset {
    static constexpr int size = (NumBits + 31) / 32;
    std::array<uint32_t, size> data{};

    // FIXME: constrain to integer types only
    template<typename... ArgTs>
    constexpr bitset(ArgTs... args) {
        (add(static_cast<size_t>(args)), ...);
    }

    constexpr bool operator==(bitset const & rhs) const {
        return data == rhs.data;
    }

    constexpr bool operator!=(bitset const & rhs) const {
        return !(*this == rhs);
    }

    constexpr void add(size_t i) {
        data[i / 32] |= (1 << (i % 32));
    }

    constexpr void remove(size_t i) {
        data[i / 32] &= ~(1 << (i % 32));
    }

    [[nodiscard]] constexpr bool contains(size_t i) const {
        return data[i / 32] & (1 << (i % 32));
    }

    template<typename T>
    constexpr void for_each(T const & action) const {
        // FIXME: bare loop
        for (auto i = size_t{}; i < size; i++) {
            auto const bit_index_base = i * 32;

            auto current_bits = data[i];
            // FIXME: bare loop
            while (current_bits != 0u) {
                // FIXME: take care of builtin in portable way
                auto const bit_index_offset = __builtin_ctz(current_bits);
                auto const bit_index = bit_index_base + static_cast<size_t>(bit_index_offset);
                current_bits &= ~(1 << bit_index_offset);
                action(bit_index);
            }
        }
    }

    constexpr bitset & operator&=(bitset const & rhs) {
        // FIXME: bare loop
        for (auto i = size_t{}; i < size; i++) {
            data[i] &= rhs.data[i];
        }

        return *this;
    }

    [[nodiscard]] constexpr operator bool() const {
        // FIXME: algorithms
        uint32_t value = 0u;
        for (auto i = size_t{}; i < size; i++) {
            value |= data[i];
        }
        return value != 0;
    }
};

}
