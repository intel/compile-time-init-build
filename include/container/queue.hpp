#pragma once

#include <array>
#include <cstdint>

/**
 * A circular queue implementation
 *
 * @tparam EntryType The type of elements the queue contains.
 * @tparam Capacity  Maximum amount of elements the circular queue.
 */
template <typename EntryType, std::size_t Capacity> class queue {
  private:
    std::array<EntryType, Capacity> storage{};
    std::size_t size{0};
    std::size_t putIndex{0};
    std::size_t getIndex{0};

  public:
    [[nodiscard]] constexpr auto get_size() const -> std::size_t {
        return size;
    }

    [[nodiscard]] constexpr auto is_full() const -> bool {
        return size == Capacity;
    }

    [[nodiscard]] constexpr auto is_empty() const -> bool { return size == 0; }

    constexpr void put(EntryType const &entry) {
        storage[putIndex] = entry;

        putIndex += 1;
        if (putIndex >= Capacity) {
            putIndex = 0;
        }

        size += (size < Capacity) ? 1 : 0;
    }

    [[nodiscard]] constexpr auto get() -> EntryType {
        auto const entry = storage[getIndex];

        getIndex += 1;
        if (getIndex >= Capacity) {
            getIndex = 0;
        }

        size -= (size > 0) ? 1 : 0;
        return entry;
    }
};
