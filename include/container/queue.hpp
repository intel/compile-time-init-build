#pragma once

#include <log/log.hpp>

#include <array>
#include <cstddef>
#include <utility>

struct unsafe_overflow_policy {
    constexpr static auto check_push(auto...) {}
    constexpr static auto check_pop(auto...) {}
};

struct safe_overflow_policy {
    constexpr static auto check_push(std::size_t size, std::size_t capacity) {
        CIB_ASSERT(size < capacity);
    }
    constexpr static auto check_pop(std::size_t size) { CIB_ASSERT(size > 0); }
};

/**
 * A circular queue implementation
 *
 * @tparam EntryType The type of elements the queue contains.
 * @tparam Capacity  Maximum amount of elements in the circular queue.
 * @tparam OverflowPolicy  Whether to check over/underflow.
 */
template <typename EntryType, std::size_t Capacity,
          typename OverflowPolicy = safe_overflow_policy>
class queue {
  private:
    std::array<EntryType, Capacity> storage{};
    std::size_t size_{};
    std::size_t push_index{Capacity - 1};
    std::size_t pop_index{};

  public:
    [[nodiscard]] constexpr auto size() const -> std::size_t { return size_; }
    [[nodiscard]] constexpr static auto capacity() -> std::size_t {
        return Capacity;
    }
    [[nodiscard]] constexpr auto full() const -> bool {
        return size_ == Capacity;
    }
    [[nodiscard]] constexpr auto empty() const -> bool { return size_ == 0u; }

    [[nodiscard]] constexpr auto front() & -> EntryType & {
        OverflowPolicy::check_pop(size_);
        return storage[pop_index];
    }
    [[nodiscard]] constexpr auto front() const & -> EntryType const & {
        OverflowPolicy::check_pop(size_);
        return storage[pop_index];
    }
    [[nodiscard]] constexpr auto back() & -> EntryType & {
        OverflowPolicy::check_pop(size_);
        return storage[push_index];
    }
    [[nodiscard]] constexpr auto back() const & -> EntryType const & {
        OverflowPolicy::check_pop(size_);
        return storage[push_index];
    }

    constexpr auto push(EntryType const &entry) -> EntryType & {
        OverflowPolicy::check_push(size_, Capacity);
        if (++push_index == Capacity) {
            push_index = 0;
        }
        ++size_;
        return storage[push_index] = entry;
    }

    constexpr auto push(EntryType &&entry) -> EntryType & {
        OverflowPolicy::check_push(size_, Capacity);
        if (++push_index == Capacity) {
            push_index = 0;
        }
        ++size_;
        return storage[push_index] = std::move(entry);
    }

    [[nodiscard]] constexpr auto pop() -> EntryType {
        OverflowPolicy::check_pop(size_);
        auto entry = std::move(storage[pop_index++]);
        if (pop_index == Capacity) {
            pop_index = 0;
        }
        --size_;
        return entry;
    }
};
