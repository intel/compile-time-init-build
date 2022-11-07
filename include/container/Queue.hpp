#pragma once

#include <cstddef>

template <typename EntryType, std::size_t Capacity> class Queue {
  private:
    EntryType storage[Capacity]{};
    std::size_t size{};
    std::size_t putIndex{};
    std::size_t getIndex{};

  public:
    [[nodiscard]] constexpr auto getSize() const -> std::size_t { return size; }

    [[nodiscard]] constexpr auto isFull() const -> bool {
        return size == Capacity;
    }

    [[nodiscard]] constexpr auto isEmpty() const -> bool { return size == 0; }

    constexpr auto put(EntryType const &entry) -> void {
        storage[putIndex] = entry;

        putIndex += 1;
        if (putIndex >= Capacity) {
            putIndex = 0;
        }

        size += 1;
    }

    [[nodiscard]] constexpr auto get() -> EntryType {
        auto const entry = storage[getIndex];

        getIndex += 1;
        if (getIndex >= Capacity) {
            getIndex = 0;
        }

        size -= 1;
        return entry;
    }
};
