#pragma once

#include <log/log.hpp>
#include <sc/string_constant.hpp>

#include <cstddef>
#include <initializer_list>
#include <iterator>

/**
 * A static array implementation that enforces bounds-checking.
 *
 * @tparam ValueType The type of elements the Array contains.
 * @tparam Size The size of the array in number of elements.
 */
template <typename ValueType, std::size_t Size> class Array {
  private:
    ValueType storage[Size]{};

    [[nodiscard]] constexpr friend auto operator==(Array const &lhs,
                                                   Array const &rhs) -> bool {
        for (int i = 0; i < Size; i++) {
            if (lhs.storage[i] != rhs.storage[i]) {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] constexpr friend auto operator!=(Array const &lhs,
                                                   Array const &rhs) -> bool {
        return not(lhs == rhs);
    }

  public:
    using value_type = ValueType;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = ValueType &;
    using const_reference = const ValueType &;
    using pointer = ValueType *;
    using const_pointer = const ValueType *;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_terator = std::reverse_iterator<const_iterator>;

    constexpr Array(std::initializer_list<ValueType> src) {
        if (src.size() > Size) {
            FATAL("Initializer list size {} is bigger than allocated array "
                  "size {}",
                  src.size(), sc::int_<Size>);
        } else {
            auto dst_index = 0;
            for (auto const &v : src) {
                storage[dst_index] = v;
                dst_index++;
            }
        }
    }

    template <typename SrcT> constexpr explicit Array(SrcT const &src) {
        if (src.size() > Size) {
            FATAL("Source size {} is bigger than allocated array size {}",
                  src.size(), sc::int_<Size>);
        } else {
            auto dst_index = 0;
            for (auto const &v : src) {
                storage[dst_index] = v;
                dst_index++;
            }
        }
    }

    constexpr Array() = default;

    [[nodiscard]] constexpr auto begin() -> iterator { return storage; }

    [[nodiscard]] constexpr auto begin() const -> const_iterator {
        return storage;
    }

    [[nodiscard]] constexpr auto end() -> iterator { return &(storage[Size]); }

    [[nodiscard]] constexpr auto end() const -> const_iterator {
        return &(storage[Size]);
    }

    [[nodiscard]] constexpr auto size() const -> std::size_t { return Size; }

    template <std::size_t Index>
    [[nodiscard]] constexpr auto get() const -> ValueType const & {
        static_assert(Index < Size, "Array index is outside array size");
        return storage[Index];
    }

    [[nodiscard]] constexpr auto operator[](std::size_t index) const
        -> ValueType const & {
        if (index >= Size) {
            FATAL("Array index {} is outside array size {}", index,
                  sc::int_<Size>);
            return storage[0];
        }
        return storage[index];
    }

    [[nodiscard]] constexpr auto operator[](std::size_t index) -> ValueType & {
        if (index >= Size) {
            FATAL("Array index {} is outside array size {}", index,
                  sc::int_<Size>);
            return storage[0];
        }
        return storage[index];
    }
};
