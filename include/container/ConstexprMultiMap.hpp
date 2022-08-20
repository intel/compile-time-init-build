#pragma once

#include <container/ConstexprMap.hpp>
#include <container/ConstexprSet.hpp>

#include <cstddef>

/**
 * A fully constexpr multi-map implementation.
 *
 * A multi-map contains one or more values per key.
 *
 * ConstexprMultiMap is perfect for compile-time initialization and
 * configuration, but its performance may not be suitable for run-time
 * operations. In particular, the current implementation has linear run-time
 * O(n) for any key lookup operation.
 *
 * ConstexprMultiMap owns the storage for the keys and values it stores.
 * Carefully consider whether the KeyType and ValueType are object values or
 * pointers.
 *
 * ConstexprMultiMap has very high storage requirements. Storage scales
 * quadratically, or O(n^2).
 *
 * Consider an appropriate Capacity for the ConstexprMultiMap. It will be
 * able to contain up to "Capacity" number of keys, and each key will be
 * able to contain up to "Capacity" number of values.
 *
 * @tparam KeyType
 * @tparam ValueType
 * @tparam KeyCapacity
 */
template<
    typename KeyType,
    typename ValueType,
    std::size_t KeyCapacity,
    std::size_t ValueCapacity = KeyCapacity>
class ConstexprMultiMap {
private:
    using StorageType =
        ConstexprMap<KeyType, ConstexprSet<ValueType, ValueCapacity>, KeyCapacity>;

    StorageType storage;

public:
    constexpr ConstexprMultiMap()
        : storage()
    {}

    /**
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      Current number of keys stored in this multi-map.
     */
    [[nodiscard]] constexpr std::size_t getSize() const {
        return storage.getSize();
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr typename StorageType::Entry* begin() {
        return storage.begin();
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr typename StorageType::Entry* end() {
        return storage.end();
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr typename StorageType::Entry const * cbegin() const {
        return storage.cbegin();
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr typename StorageType::Entry const * cend() const {
        return storage.cend();
    }

    /**
     * Put an entry into the multi-map. If the key 'k' already exists in the
     * map, the existing entry is updated with an additional value 'v'.
     *
     * Behavior is undefined if the map would exceed its Capacity for keys,
     * or its Capacity for values for key 'k'.
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
        if (storage.contains(k)) {
            storage.get(k).add(v);

        } else {
            ConstexprSet<ValueType, ValueCapacity> set;
            set.add(v);
            storage.put(k, set);
        }
    }

    /**
     * Put an entry into the multi-map. If the key 'k' already exists in the
     * map, then do nothing.
     *
     * Behavior is undefined if the map would exceed its Capacity for keys.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param k
     *      New key 'k'.
     */
    constexpr void put(KeyType k) {
        if (!storage.contains(k)) {
            ConstexprSet<ValueType, ValueCapacity> set;
            storage.put(k, set);
        }
    }

    /**
     * Remove all values from the corresponding targetKey from the multi-map.
     * If targetKey is not found, do nothing.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param targetKey
     *      The key to search for and remove.
     */
    constexpr void remove(KeyType targetKey) {
        storage.remove(targetKey);
    }

    /**
     * Remove corresponding targetKey and targetValue from the multi-map.
     * If targetKey or targetValue are not found, do nothing.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param targetKey
     *      The key to search for and remove.
     *
     * @param targetValue
     *      The value to search for and remove.
     */
    constexpr void remove(KeyType targetKey, ValueType targetValue) {
        if (storage.contains(targetKey)) {
            storage.get(targetKey).remove(targetValue);

            if (storage.get(targetKey).isEmpty()) {
                storage.remove(targetKey);
            }
        }
    }

    /**
     * Get a mutable reference to the ConstexprSet for the given targetKey.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param targetKey
     *      Key to search for.
     *
     * @return
     *      Reference to the ConstexprSet if the targetKey is found. Undefined
     *      behavior otherwise.
     */
    [[nodiscard]] constexpr ConstexprSet<ValueType, ValueCapacity> & get(KeyType k) {
        return storage.get(k);
    }

    /**
     * Get a const reference to the ConstexprSet for the given targetKey.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param targetKey
     *      Key to search for.
     *
     * @return
     *      Reference to the ConstexprSet if the targetKey is found. Undefined
     *      behavior otherwise.
     */
    [[nodiscard]] constexpr ConstexprSet<ValueType, ValueCapacity> const & get(KeyType k) const {
        return storage.get(k);
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      True if the multi-map is empty.
     */
    [[nodiscard]] constexpr bool isEmpty() const {
        return storage.isEmpty();
    }

    /**
     * Check if the multi-map contains targetKey.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param targetKey
     *      The key to search for.
     *
     * @return
     *      True if targetKey is found in the multi-map.
     */
    [[nodiscard]] constexpr bool contains(KeyType targetKey) const {
        return storage.contains(targetKey);
    }

    /**
     * Check if the multi-map contains key 'k', and value 'v' in key 'k'.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param k
     *      The key to search for.
     *
     * @param v
     *      The value to search for.
     *
     * @return
     *      True if key 'k' and value 'v' is found in the multi-map.
     */
    [[nodiscard]] constexpr bool contains(KeyType k, ValueType v) const {
        if (contains(k)) {
            return get(k).contains(v);
        } else {
            return false;
        }
    }
};

