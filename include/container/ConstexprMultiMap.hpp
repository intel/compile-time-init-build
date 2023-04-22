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
 * operations. in_t particular, the current implementation has linear run-time
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
 * able to contain up to "KeyCapacity" number of keys, and each key will be
 * able to contain up to "ValueCapacity" number of values.
 *
 * @tparam KeyType
 * @tparam ValueType
 * @tparam KeyCapacity
 * @tparam ValueCapacity
 */
template <typename KeyType, typename ValueType, std::size_t KeyCapacity,
          std::size_t ValueCapacity = KeyCapacity>
class ConstexprMultiMap {
  private:
    using StorageType =
        ConstexprMap<KeyType, ConstexprSet<ValueType, ValueCapacity>,
                     KeyCapacity>;

    StorageType storage{};

  public:
    /**
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      Current number of keys stored in this multi-map.
     */
    [[nodiscard]] constexpr auto getSize() const -> std::size_t {
        return storage.getSize();
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto begin() -> typename StorageType::Entry * {
        return storage.begin();
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto end() -> typename StorageType::Entry * {
        return storage.end();
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto cbegin() const ->
        typename StorageType::Entry const * {
        return storage.cbegin();
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto cend() const ->
        typename StorageType::Entry const * {
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
    constexpr auto put(KeyType k, ValueType v) -> void {
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
    constexpr auto put(KeyType k) -> void {
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
    constexpr auto remove(KeyType targetKey) -> void {
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
    constexpr auto remove(KeyType targetKey, ValueType targetValue) -> void {
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
    [[nodiscard]] constexpr auto get(KeyType k)
        -> ConstexprSet<ValueType, ValueCapacity> & {
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
    [[nodiscard]] constexpr auto get(KeyType k) const
        -> ConstexprSet<ValueType, ValueCapacity> const & {
        return storage.get(k);
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      True if the multi-map is empty.
     */
    [[nodiscard]] constexpr auto isEmpty() const -> bool {
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
    [[nodiscard]] constexpr auto contains(KeyType targetKey) const -> bool {
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
    [[nodiscard]] constexpr auto contains(KeyType k, ValueType v) const
        -> bool {
        return contains(k) and get(k).contains(v);
    }
};
