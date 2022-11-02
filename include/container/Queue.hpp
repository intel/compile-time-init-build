#pragma once

#include <cstdint>

template <typename EntryType, std::size_t Capacity> class Queue {
  private:
    EntryType storage[Capacity];
    std::size_t size;
    std::size_t putIndex;
    std::size_t getIndex;

  public:
    constexpr Queue() : storage{}, size{0}, putIndex{0}, getIndex{0} {}

    [[nodiscard]] constexpr std::size_t getSize() const { return size; }

    [[nodiscard]] constexpr bool isFull() const { return size == Capacity; }

    [[nodiscard]] constexpr bool isEmpty() const { return size == 0; }

    constexpr void put(EntryType const &entry) {
        storage[putIndex] = entry;

        putIndex += 1;
        if (putIndex >= Capacity) {
            putIndex = 0;
        }

        size += 1;
    }

    [[nodiscard]] constexpr EntryType get() {
        auto const entry = storage[getIndex];

        getIndex += 1;
        if (getIndex >= Capacity) {
            getIndex = 0;
        }

        size -= 1;
        return entry;
    }
};
