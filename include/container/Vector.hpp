#pragma once

#include <log/log.hpp>

#include <cstddef>
#include <initializer_list>
#include <iterator>

using size_t = std::size_t;

/**
 * A vector implementation that enforces bounds-checking.
 *
 * @tparam ValueType The type of elements the Vector contains.
 * @tparam Capacity  Maximum amount of elements the Vector can hold.
 */
template<typename ValueType, size_t Capacity>
class Vector {
private:
    ValueType storage[Capacity];
    size_t    currentSize;

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

    constexpr Vector(std::initializer_list<ValueType> src)
        : storage{}
        , currentSize(0)
    {
        if (src.size() > Capacity) {
            FATAL(
                "Initializer list size {} is bigger than vector capacity {}",
                src.size(), sc::int_<Capacity>);

        } else {
            auto i = 0;
            for (auto element : src) {
                storage[i] = element;
                i++;
            }

            currentSize = src.size();
        }
    }

    constexpr Vector()
        : storage{}
        , currentSize(0)
    {
    }

    constexpr Vector(Vector const &rhs) = default;
    constexpr Vector &operator=(Vector const &rhs) = default;
    constexpr Vector(Vector &&rhs) = default;
    constexpr Vector &operator=(Vector &&rhs) = default;

    [[nodiscard]] constexpr iterator begin() {
        return storage;
    }

    [[nodiscard]] constexpr const_iterator begin() const {
        return storage;
    }

    [[nodiscard]] constexpr iterator end() {
        return &(storage[currentSize]);
    }

    [[nodiscard]] constexpr const_iterator end() const {
        return &(storage[currentSize]);
    }

    [[nodiscard]] constexpr std::uint32_t size() const {
        return currentSize;
    }

    [[nodiscard]] constexpr std::size_t getCapacity() const {
        return Capacity;
    }

    [[nodiscard]] constexpr const ValueType& operator[](std::size_t index) const {
        if (index >= currentSize) {
            FATAL("Vector index {} is outside vector size {}", index, currentSize);
            return storage[0];
        } else {
            return storage[index];
        }
    }

    [[nodiscard]] constexpr ValueType& operator[](std::size_t index) {
        if (index >= currentSize) {
            FATAL("Vector index {} is outside array size {}", index, currentSize);
            return storage[0];
        } else {
            return storage[index];
        }
    }

    [[nodiscard]] constexpr bool isFull() const {
        return (currentSize == Capacity);
    }

    [[nodiscard]] constexpr bool isEmpty() const {
        return (currentSize == 0);
    }

    constexpr void push(ValueType value) {
        if(isFull()) {
            FATAL("Vector:push() attempted when full");
        } else {
            storage[currentSize] = value;
            currentSize++;
        }
    }

    [[nodiscard]] constexpr ValueType pop() {
        if (isEmpty()) {
            FATAL("Vector::pop() attempted when empty");
            return {};
        } else {
            currentSize--;
            return storage[currentSize];
        }
    }

    [[nodiscard]] constexpr bool operator==(Vector const & rhs) const {
        if (size() != rhs.size()) {
            return false;

        } else {
            for (auto i = 0; i < size(); i++) {
                if ((*this)[i] != rhs[i]) {
                    return false;
                }
            }
        }

        return true;
    }

    [[nodiscard]] constexpr bool operator!=(Vector const & rhs) const {
        return !((*this) == rhs);
    }

    [[nodiscard]] constexpr Vector operator+(Vector const & rhs) const {
        Vector result = *this;

        for (auto i = rhs.begin(); i != rhs.end(); i++) {
            result.push(*i);
        }

        return result;
    }
};
