#pragma once

#include <log/log.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <optional>
#include <utility>

/**
 * A single (key, value) entry in the map.
 *
 * @tparam KeyType
 * @tparam ValueType
 */
template <typename KeyType, typename ValueType> struct MapEntry {
    KeyType key{};
    ValueType value{};

    constexpr MapEntry(KeyType k, ValueType v) : key(k), value(v) {}

    constexpr MapEntry() = default;
};

/**
 * A fully constexpr map implementation.
 *
 * ConstexprMap is perfect for compile-time initialization and configuration,
 * but its performance may not be suitable for run-time operations. in_t
 * particular, the current implementation has linear run-time O(n) for any key
 * lookup operation.
 *
 * ConstexprMap owns the storage for the keys and values it stores. Carefully
 * consider whether the KeyType and ValueType are object values or pointers.
 *
 * Consider an appropriate Capacity for the ConstexprMap. It will be able to
 * contain up to "Capacity" number of entries.
 *
 * @tparam KeyType
 * @tparam ValueType
 * @tparam Capacity
 */
template <typename KeyType, typename ValueType, std::size_t Capacity>
class ConstexprMap {
  public:
    using Entry = MapEntry<KeyType, ValueType>;

  private:
    std::array<Entry, Capacity> storage{};
    std::size_t size{};

    /**
     * Find the storage index of the given key.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param targetKey
     *      The key to look for.
     *
     * @return
     *      Index into storage where the corresponding entry is stored. If
     *      the targetKey cannot be found, then std::nullopt is returned.
     */
    [[nodiscard]] constexpr auto find(KeyType targetKey) const
        -> std::optional<std::size_t> {
        for (auto i = std::size_t{}; i < size; ++i) {
            if (storage[i].key == targetKey) {
                return i;
            }
        }

        return {};
    }

  public:
    constexpr ConstexprMap() = default;

    template <typename T> constexpr explicit ConstexprMap(T const &rhs) {
        for (auto const &entry : std::as_const(rhs)) {
            put(entry.key, entry.value);
        }
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      Current number of entries stored in this map.
     */
    [[nodiscard]] constexpr auto getSize() const -> std::size_t { return size; }

    /**
     * Remove an unspecified entry from the map.
     *
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      An Entry from the map.
     */
    [[nodiscard]] constexpr auto pop() -> Entry {
        CIB_ASSERT(size > 0);
        return storage[--size];
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      True if the map is empty.
     */
    [[nodiscard]] constexpr auto isEmpty() const -> bool { return size == 0; }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto begin() -> Entry * {
        return std::begin(storage);
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto end() -> Entry * {
        return std::begin(storage) + size;
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto begin() const -> Entry const * {
        return std::cbegin(storage);
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto end() const -> Entry const * {
        return std::cbegin(storage) + size;
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
    [[nodiscard]] constexpr auto get(KeyType targetKey) -> ValueType & {
        auto entryIndex = find(targetKey);
        CIB_ASSERT(entryIndex);
        return storage[*entryIndex].value;
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
    [[nodiscard]] constexpr auto get(KeyType targetKey) const
        -> ValueType const & {
        auto entryIndex = find(targetKey);
        CIB_ASSERT(entryIndex);
        return storage[*entryIndex].value;
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
    constexpr auto put(KeyType k, ValueType v) -> void {
        auto entryIndex = find(k);

        if (entryIndex) {
            storage[*entryIndex].value = v;
        } else {
            CIB_ASSERT(size < Capacity);
            storage[size++] = Entry(k, v);
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
    [[nodiscard]] constexpr auto contains(KeyType targetKey) const -> bool {
        return static_cast<bool>(find(targetKey));
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
    constexpr auto remove(KeyType targetKey) -> void {
        auto entryIndex = find(targetKey);
        if (entryIndex) {
            storage[*entryIndex] = storage[--size];
        }
    }
};
