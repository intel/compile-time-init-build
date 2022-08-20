#pragma once

#include <container/ConstexprMap.hpp>

#include <cstddef>

/**
 * A fully constexpr set implementation.
 *
 * ConstexprSet is perfect for compile-time initialization and configuration,
 * but its performance may not be suitable for run-time operations. In
 * particular, the current implementation has linear run-time O(n) for any key
 * lookup operation.
 *
 * ConstexprSet owns the storage for the keys it stores. Carefully consider
 * whether the KeyType is an object value or pointer.
 *
 * Consider an appropriate Capacity for the ConstexprSet. It will be able to
 * contain up to "Capacity" number of keys.
 *
 * @tparam KeyType
 * @tparam Capacity
 */
template<
    typename KeyType,
    std::size_t Capacity>
class ConstexprSet {
private:
    using StorageType = ConstexprMap<KeyType, bool, Capacity>;
    StorageType storage;

public:
    constexpr ConstexprSet()
        : storage()
    {}

    constexpr ConstexprSet(std::initializer_list<KeyType> src)
            : storage()
    {
        if (src.size() > Capacity) {
            FATAL("Initializer list size {} is bigger than allocated set capacity {}", src.size(), Capacity);
        }
        else {
            for (auto k : src) {
                storage.put(k, true);
            }
        }
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      Current number of keys stored in this set.
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
    [[nodiscard]] constexpr typename StorageType::Entry const * begin() const {
        return storage.begin();
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr typename StorageType::Entry const * end() const {
        return storage.end();
    }

    /**
     * Check if the set contains targetKey.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param targetKey
     *      The key to search for.
     *
     * @return
     *      True if targetKey is found in the set.
     */
    [[nodiscard]] constexpr bool contains(KeyType targetKey) const {
        return storage.contains(targetKey);
    }

    /**
     * Put a key into the map. If the key 'k' already exists in the map, then
     * no action is performed.
     *
     * <b>Runtime complexity:</b> O(n)
     *
     * @param k
     *      New key 'k' to add to the set.
     */
    constexpr void add(KeyType k) {
        storage.put(k, true);
    }

    /**
     * Remove targetKey from the map. If targetKey is not found, do nothing.
     *
     * <b>Runtime complexity:</b> O(n * m)
     *
     * @param targetKey
     *      The key to search for and remove.
     */
    constexpr void remove(KeyType targetKey) {
        storage.remove(targetKey);
    }

    /**
     * Add all keys from 'addSet' into this set.
     *
     * <b>Runtime complexity:</b> O(n * m)
     *
     * @param addSet
     */
    template<typename RhsSetType>
    constexpr void addAll(RhsSetType addSet) {
        for (auto entry : addSet) {
            add(entry.key);
        }
    }

    /**
     * Remove all keys in 'removeSet' from this set.
     *
     * <b>Runtime complexity:</b> O(n * m)
     *
     * @param removeSet
     */
    template<typename RhsSetType>
    constexpr void removeAll(RhsSetType removeSet) {
        for (auto entry : removeSet) {
            remove(entry.key);
        }
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      True if the set is empty.
     */
    [[nodiscard]] constexpr bool isEmpty() const {
        return storage.isEmpty();
    }

    /**
     * Remove an unspecified key from the set.
     *
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      A key from the set.
     */
    [[nodiscard]] constexpr KeyType pop() {
        return storage.pop().key;
    }
};


