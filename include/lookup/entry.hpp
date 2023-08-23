#pragma once

namespace lookup {
template <typename K, typename V> struct entry {
    K key_;
    V value_;

    template <typename Kp, typename Vp>
    constexpr entry(entry<Kp, Vp> const &rhs)
        : key_{rhs.key_}, value_{rhs.value_} {}

    constexpr entry(K key, V value) : key_{key}, value_{value} {}

    constexpr entry() : key_{}, value_{} {}
};
} // namespace lookup
