#pragma once

#include <log/log.hpp>

#include <iterator>
#include <cstddef>
#include <optional>
#include <utility>

/**
 * A single (key, value) entry in the map.
 *
 * @tparam KeyType
 * @tparam ValueType
 */
template<
    typename KeyType,
    typename ValueType>
struct MapEntry {
    KeyType key;
    ValueType value;

    constexpr MapEntry(
        KeyType k,
        ValueType v
    )
        : key(k)
        , value(v)
    {}

    constexpr MapEntry()
        : key()
        , value()
    {}
};

/**
 * A fully constexpr map implementation.
 *
 * ConstexprMap is perfect for compile-time initialization and configuration,
 * but its performance may not be suitable for run-time operations. In
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
template<
    typename KeyType,
    typename ValueType,
    std::size_t Capacity>
class ConstexprMap {
public:
    using Entry = MapEntry<KeyType, ValueType>;

private:
    Entry storage[Capacity];
    std::size_t size;

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
     *      the targetKey cannot be found, then -1 is returned.
     */
    [[nodiscard]] constexpr std::optional<std::size_t> find(KeyType targetKey) const {
        for (std::size_t i = 0; i < size; i++) {
            if (storage[i].key == targetKey) {
                return i;
            }
        }

        return {};
    }

public:
    constexpr ConstexprMap()
        : storage()
        , size(0)
    {}

    template<typename T>
    constexpr ConstexprMap(T const & rhs)
        : storage()
        , size(0)
    {
        for (auto const & entry : std::as_const(rhs)) {
            put(entry.key, entry.value);
        }
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      Current number of entries stored in this map.
     */
    [[nodiscard]] constexpr std::size_t getSize() const {
        return size;
    }

    /**
     * Remove an unspecified entry from the map.
     *
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      An Entry from the map.
     */
    [[nodiscard]] constexpr Entry pop() {
        ASSERT(size > 0);
        size--;
        return storage[size];
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      True if the map is empty.
     */
    [[nodiscard]] constexpr bool isEmpty() const {
        return size == 0;
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr Entry* begin() {
        return std::begin(storage);
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr Entry* end() {
        return &(storage[size]);
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr Entry const * begin() const {
        return std::cbegin(storage);
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr Entry const * end() const {
        return &(storage[size]);
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
    [[nodiscard]] constexpr ValueType & get(KeyType targetKey) {
        auto entryIndex = find(targetKey);
        ASSERT(entryIndex);
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
    [[nodiscard]] constexpr ValueType const & get(KeyType targetKey) const {
        auto entryIndex = find(targetKey);
        ASSERT(entryIndex);
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
    constexpr void put(KeyType k, ValueType v) {
        auto entryIndex = find(k);

        if (entryIndex) {
            storage[*entryIndex].value = v;
        } else {
            ASSERT(size < Capacity);
            storage[size] = Entry(k, v);
            size++;
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
    [[nodiscard]] constexpr bool contains(KeyType targetKey) const {
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
    constexpr void remove(KeyType targetKey) {
        auto entryIndex = find(targetKey);

        if (entryIndex) {
            storage[*entryIndex] = storage[size - 1];
            size--;
        }
    }
};
