#pragma once

#include <container/constexpr_map.hpp>

#include <concepts>
#include <cstddef>

namespace cib {
/**
 * A fully constexpr set implementation.
 *
 * constexpr_set is perfect for compile-time initialization and configuration,
 * but its performance may not be suitable for run-time operations. in_t
 * particular, the current implementation has linear run-time O(n) for any key
 * lookup operation.
 *
 * constexpr_set owns the storage for the keys it stores. Carefully consider
 * whether the KeyType is an object value or pointer.
 *
 * Consider an appropriate Capacity for the constexpr_set. It will be able to
 * contain up to "Capacity" number of keys.
 *
 * @tparam KeyType
 * @tparam Capacity
 */
template <typename KeyType, std::size_t Capacity> class constexpr_set {
    using StorageType = constexpr_map<KeyType, bool, Capacity>;
    StorageType storage{};

  public:
    using key_type = KeyType;
    using value_type = KeyType;
    using reference = value_type &;
    using const_reference = value_type const &;

    template <typename... Ts>
        requires((sizeof...(Ts) <= Capacity) and ... and
                 std::convertible_to<key_type, Ts>)
    constexpr explicit(true) constexpr_set(Ts const &...ts)
        : storage{typename StorageType::value_type{static_cast<key_type>(ts),
                                                   true}...} {}

    /**
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      Current number of keys stored in this set.
     */
    [[nodiscard]] constexpr auto size() const -> std::size_t {
        return storage.size();
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto begin() -> typename StorageType::iterator {
        return storage.begin();
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto end() -> typename StorageType::iterator {
        return storage.end();
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto begin() const ->
        typename StorageType::const_iterator {
        return storage.begin();
    }

    /**
     * <b>Runtime complexity:</b> O(1)
     */
    [[nodiscard]] constexpr auto end() const ->
        typename StorageType::const_iterator {
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
    [[nodiscard]] constexpr auto contains(key_type targetKey) const -> bool {
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
    constexpr void add(key_type k) { storage.put(k, true); }

    /**
     * Remove targetKey from the map. If targetKey is not found, do nothing.
     *
     * <b>Runtime complexity:</b> O(n * m)
     *
     * @param targetKey
     *      The key to search for and remove.
     */
    constexpr void remove(key_type targetKey) { storage.remove(targetKey); }

    /**
     * Add all keys from 'addSet' into this set.
     *
     * <b>Runtime complexity:</b> O(n * m)
     *
     * @param addSet
     */
    template <typename RhsSetType> constexpr void add_all(RhsSetType addSet) {
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
    template <typename RhsSetType>
    constexpr void remove_all(RhsSetType removeSet) {
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
    [[nodiscard]] constexpr auto empty() const -> bool {
        return storage.empty();
    }

    /**
     * Remove an unspecified key from the set.
     *
     * <b>Runtime complexity:</b> O(1)
     *
     * @return
     *      A key from the set.
     */
    [[nodiscard]] constexpr auto pop() -> key_type { return storage.pop().key; }
};

template <typename T, typename... Ts>
constexpr_set(T, Ts...) -> constexpr_set<T, 1 + sizeof...(Ts)>;
} // namespace cib
