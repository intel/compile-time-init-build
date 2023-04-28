#pragma once

#include <log/log.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <iterator>

namespace cib {
/**
 * A fully constexpr map implementation.
 *
 * constexpr_map is perfect for compile-time initialization and configuration,
 * but its performance may not be suitable for run-time operations. in_t
 * particular, the current implementation has linear run-time O(n) for any key
 * lookup operation.
 *
 * constexpr_map owns the storage for the keys and values it stores. Carefully
 * consider whether the KeyType and ValueType are object values or pointers.
 *
 * Consider an appropriate Capacity for the constexpr_map. It will be able to
 * contain up to "Capacity" number of entries.
 *
 * @tparam KeyType
 * @tparam ValueType
 * @tparam Capacity
 */
template <typename KeyType, typename ValueType, std::size_t Capacity>
class constexpr_map {
  public:
    using key_type = KeyType;
    using mapped_type = ValueType;
    struct value_type {
        key_type key{};
        mapped_type value{};
    };
    using reference = value_type &;
    using const_reference = value_type const &;
    using iterator = value_type *;
    using const_iterator = value_type const *;

  private:
    std::array<value_type, Capacity> storage{};
    std::size_t current_size{};

  public:
    template <typename... Vs>
        requires((sizeof...(Vs) <= Capacity) and ... and
                 std::same_as<value_type, Vs>)
    constexpr explicit(true) constexpr_map(Vs const &...vs)
        : storage{vs...}, current_size{sizeof...(Vs)} {}

    /**
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      Current number of entries stored in this map.
     */
    [[nodiscard]] constexpr auto size() const -> std::size_t {
        return current_size;
    }

    /**
     * Remove an unspecified entry from the map.
     *
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      An Entry from the map.
     */
    [[nodiscard]] constexpr auto pop() -> value_type {
        CIB_ASSERT(current_size > 0);
        return storage[--current_size];
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      True if the map is empty.
     */
    [[nodiscard]] constexpr auto empty() const -> bool {
        return current_size == 0;
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto begin() -> iterator {
        return std::begin(storage);
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto end() -> iterator {
        return std::begin(storage) + current_size;
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto begin() const -> const_iterator {
        return std::cbegin(storage);
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto end() const -> const_iterator {
        return std::cbegin(storage) + current_size;
    }

    /**
     * Get a mutable reference to the value for the given targetKey.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param targetKey
     *      Key to search for.
     *
     * @return
     *      Reference to the value if the targetKey is found.
     */
    [[nodiscard]] constexpr auto get(key_type targetKey) -> mapped_type & {
        auto const i = std::find_if(begin(), end(), [&](value_type const &v) {
            return v.key == targetKey;
        });
        CIB_ASSERT(i != end());
        return i->value;
    }

    /**
     * Get a const reference to the value for the given targetKey.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param targetKey
     *      Key to search for.
     *
     * @return
     *      Reference to the value if the targetKey is found.
     */
    [[nodiscard]] constexpr auto get(key_type targetKey) const
        -> mapped_type const & {
        auto const i = std::find_if(begin(), end(), [&](value_type const &v) {
            return v.key == targetKey;
        });
        CIB_ASSERT(i != end());
        return i->value;
    }

    /**
     * Put an entry into the map. If the key 'k' already exists in the map, the
     * existing entry is updated with the new value 'v'.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param k
     *      New key 'k' to associate with value 'v'.
     *
     * @param v
     *      New value 'v' for key 'k'.
     */
    constexpr auto put(key_type k, mapped_type v) -> void {
        if (auto const i = std::find_if(
                begin(), end(),
                [&](value_type const &elem) { return elem.key == k; });
            i != end()) {
            i->value = v;
        } else {
            CIB_ASSERT(current_size < Capacity);
            storage[current_size++] = {k, v};
        }
    }

    /**
     * Check if the map contains targetKey.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param targetKey
     *      The key to search for.
     *
     * @return
     *      True if targetKey is found in the map.
     */
    [[nodiscard]] constexpr auto contains(key_type targetKey) const -> bool {
        return std::find_if(begin(), end(), [&](value_type const &v) {
                   return v.key == targetKey;
               }) != end();
    }

    /**
     * Remove the entry corresponding to targetKey from the map. If targetKey
     * is not found, do nothing.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param targetKey
     *      The key to search for and remove.
     */
    constexpr auto remove(key_type targetKey) -> void {
        if (auto i = std::find_if(
                begin(), end(),
                [&](value_type const &v) { return v.key == targetKey; });
            i != end()) {
            *i = storage[--current_size];
        }
    }
};
} // namespace cib
