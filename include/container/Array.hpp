#pragma once

#include <Logger.hpp>
#include <StringCatalog.hpp>

#include <cstddef>
#include <initializer_list>
#include <iterator>

/**
 * A static array implementation that enforces bounds-checking.
 *
 * @tparam ValueType The type of elements the Array contains.
 * @tparam Size The size of the array in number of elements.
 */
template<
    typename ValueType,
    std::size_t Size>
class Array {
private:
    ValueType storage[Size];

public:
    using value_type = ValueType;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = ValueType&;
    using const_reference = const ValueType&;
    using pointer = ValueType*;
    using const_pointer = const ValueType*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_terator = std::reverse_iterator<const_iterator>;

    constexpr Array(std::initializer_list<ValueType> src)
        : storage{}
    {
        if (src.size() > Size) {
            FATAL(
                "Initializer list size {} is bigger than allocated array size {}",
                src.size(), int_<Size>);

        } else {
            auto dst_index = 0;
            for (auto v : src) {
                storage[dst_index] = v;
                dst_index++;
            }
        }
    }

    template<typename SrcT>
    constexpr Array(SrcT const & src)
        : storage{}
    {
        if (src.size() > Size) {
            FATAL(
                "Source size {} is bigger than allocated array size {}",
                src.size(), int_<Size>);

        } else {
            auto dst_index = 0;
            for (auto v : src) {
                storage[dst_index] = v;
                dst_index++;
            }
        }
    }

    constexpr Array()
        : storage{}
    {
        // pass
    }

    constexpr Array(Array const & rhs) = default;
    constexpr Array &operator=(Array const & rhs) = default;
    constexpr Array(Array && rhs) = default;
    constexpr Array &operator=(Array && rhs) = default;

    [[nodiscard]] constexpr iterator begin() {
        return storage;
    }

    [[nodiscard]] constexpr const_iterator begin() const {
        return storage;
    }

    [[nodiscard]] constexpr iterator end() {
        return &(storage[Size]);
    }

    [[nodiscard]] constexpr const_iterator end() const {
        return &(storage[Size]);
    }

    [[nodiscard]] constexpr std::size_t size() const {
        return Size;
    }

    template<std::size_t Index>
    [[nodiscard]] constexpr const ValueType & get() const {
        static_assert(Index < Size, "Array index is outside array size");
        return storage[Index];
    }


    [[nodiscard]] constexpr const ValueType & operator[](std::size_t index) const {
        if (index >= Size) {
            FATAL(
                "Array index {} is outside array size {}",
                index, int_<Size>);

            return storage[0];

        } else {
            return storage[index];
        }
    }


    [[nodiscard]] constexpr ValueType & operator[](std::size_t index) {
        if (index >= Size) {
            FATAL(
                "Array index {} is outside array size {}",
                index, int_<Size>);

            return storage[0];

        } else {
            return storage[index];
        }
    }


    [[nodiscard]] constexpr bool operator==(Array const & rhs) const {
        for (int i = 0; i < Size; i++) {
            if (storage[i] != rhs.storage[i]) {
                return false;
            }
        }

        return true;
    }


    [[nodiscard]] constexpr bool operator!=(Array const & rhs) const {
        return !(this->operator==(rhs));
    }
};

