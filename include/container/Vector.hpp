#pragma once

#include <log/log.hpp>

#include <array>
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
template <typename ValueType, size_t Capacity> class Vector {
    std::array<ValueType, Capacity> storage{};
    std::size_t currentSize{};

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

    constexpr Vector(std::initializer_list<ValueType> src) {
        if (src.size() > Capacity) {
            CIB_FATAL(
                "Initializer list size {} is bigger than vector capacity {}",
                src.size(), sc::int_<Capacity>);
        } else {
            auto i = std::size_t{};
            for (auto element : src) {
                storage[i] = element;
                i++;
            }

            currentSize = src.size();
        }
    }

    constexpr Vector() = default;

    [[nodiscard]] constexpr auto begin() -> iterator { return storage.data(); }

    [[nodiscard]] constexpr auto begin() const -> const_iterator {
        return storage.data();
    }

    [[nodiscard]] constexpr auto end() -> iterator {
        return &storage[currentSize];
    }

    [[nodiscard]] constexpr auto end() const -> const_iterator {
        return &storage[currentSize];
    }

    [[nodiscard]] constexpr auto size() const -> std::size_t {
        return currentSize;
    }

    [[nodiscard]] constexpr auto getCapacity() const -> std::size_t {
        return Capacity;
    }

    [[nodiscard]] constexpr auto operator[](std::size_t index) const
        -> ValueType const & {
        if (index >= currentSize) {
            CIB_FATAL("Vector index {} is outside vector size {}", index,
                      currentSize);
            return storage[0];
        }
        return storage[index];
    }

    [[nodiscard]] constexpr auto operator[](std::size_t index) -> ValueType & {
        if (index >= currentSize) {
            CIB_FATAL("Vector index {} is outside array size {}", index,
                      currentSize);
            return storage[0];
        }
        return storage[index];
    }

    [[nodiscard]] constexpr auto isFull() const -> bool {
        return (currentSize == Capacity);
    }

    [[nodiscard]] constexpr auto isEmpty() const -> bool {
        return (currentSize == 0);
    }

    constexpr auto push(ValueType value) -> void {
        if (isFull()) {
            CIB_FATAL("Vector::push() attempted when full");
            return;
        }
        storage[currentSize] = value;
        currentSize++;
    }

    [[nodiscard]] constexpr auto pop() -> ValueType {
        if (isEmpty()) {
            CIB_FATAL("Vector::pop() attempted when empty");
            return {};
        }
        currentSize--;
        return storage[currentSize];
    }

    [[nodiscard]] constexpr auto operator==(Vector const &rhs) const -> bool {
        if (size() != rhs.size()) {
            return false;
        }
        for (auto i = std::size_t{}; i < size(); i++) {
            if ((*this)[i] != rhs[i]) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr auto operator!=(Vector const &rhs) const -> bool {
        return !((*this) == rhs);
    }

    [[nodiscard]] constexpr auto operator+(Vector const &rhs) const -> Vector {
        Vector result = *this;

        for (auto i = rhs.begin(); i != rhs.end(); i++) {
            result.push(*i);
        }

        return result;
    }
};

template <typename T, typename... Ts>
Vector(T, Ts...) -> Vector<T, 1 + sizeof...(Ts)>;
