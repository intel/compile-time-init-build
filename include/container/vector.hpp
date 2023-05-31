#pragma once

#include <log/log.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <iterator>

namespace cib {
/**
 * A vector implementation that enforces bounds-checking.
 *
 * @tparam ValueType The type of elements the vector contains.
 * @tparam Capacity  Maximum amount of elements the vector can hold.
 */
template <typename ValueType, std::size_t Capacity> class vector {
  protected:
    // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
    std::array<ValueType, Capacity> storage{};
    // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
    std::size_t current_size{};

  public:
    using value_type = ValueType;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = ValueType &;
    using const_reference = ValueType const &;
    using pointer = ValueType *;
    using const_pointer = ValueType const *;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_terator = std::reverse_iterator<const_iterator>;

    template <typename... Ts>
        requires((sizeof...(Ts) <= Capacity) and ... and
                 std::convertible_to<ValueType, Ts>)
    constexpr explicit(true) vector(Ts const &...ts)
        : storage{static_cast<ValueType>(ts)...}, current_size{sizeof...(Ts)} {}

    [[nodiscard]] constexpr auto begin() -> iterator {
        return std::begin(storage);
    }

    [[nodiscard]] constexpr auto begin() const -> const_iterator {
        return std::cbegin(storage);
    }

    [[nodiscard]] constexpr auto end() -> iterator {
        return std::begin(storage) + current_size;
    }

    [[nodiscard]] constexpr auto end() const -> const_iterator {
        return std::cbegin(storage) + current_size;
    }

    [[nodiscard]] constexpr auto size() const -> std::size_t {
        return current_size;
    }

    [[nodiscard]] constexpr auto capacity() const -> std::size_t {
        return Capacity;
    }

    [[nodiscard]] constexpr auto operator[](std::size_t index) const
        -> ValueType const & {
        if (index >= current_size) {
            CIB_FATAL("vector index {} is outside vector size {}", index,
                      current_size);
            return storage[0];
        }
        return storage[index];
    }

    [[nodiscard]] constexpr auto operator[](std::size_t index) -> ValueType & {
        if (index >= current_size) {
            CIB_FATAL("vector index {} is outside array size {}", index,
                      current_size);
            return storage[0];
        }
        return storage[index];
    }

    template <std::size_t Index>
    [[nodiscard]] constexpr auto get() const -> ValueType const & {
        return std::get<Index>(storage);
    }

    template <std::size_t Index>
    [[nodiscard]] constexpr auto get() -> ValueType & {
        return std::get<Index>(storage);
    }

    [[nodiscard]] constexpr auto full() const -> bool {
        return current_size == Capacity;
    }

    [[nodiscard]] constexpr auto empty() const -> bool {
        return current_size == 0;
    }

    constexpr auto push_back(ValueType value) -> void {
        if (full()) {
            CIB_FATAL("vector::push_back() attempted when full");
            return;
        }
        storage[current_size++] = value;
    }

    [[nodiscard]] constexpr auto pop_back() -> ValueType {
        if (empty()) {
            CIB_FATAL("vector::pop_back() attempted when empty");
            return {};
        }
        return storage[--current_size];
    }

  private:
    [[nodiscard]] friend constexpr auto operator==(vector const &lhs,
                                                   vector const &rhs) -> bool {
        return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs),
                          std::end(rhs));
    }

    [[nodiscard]] friend constexpr auto operator+(vector const &lhs,
                                                  vector const &rhs) -> vector {
        auto result = lhs;
        std::copy(std::begin(rhs), std::end(rhs), std::back_inserter(result));
        return result;
    }
};

template <typename T, typename... Ts>
vector(T, Ts...) -> vector<T, 1 + sizeof...(Ts)>;

template <std::size_t I, typename T, std::size_t N>
auto get(vector<T, N> &v) -> decltype(auto) {
    return v.template get<I>();
}

template <std::size_t I, typename T, std::size_t N>
auto get(vector<T, N> const &v) -> decltype(auto) {
    return v.template get<I>();
}

} // namespace cib
