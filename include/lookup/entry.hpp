#pragma once

namespace lookup {
template <typename K, typename V> struct entry {
    K key_{};
    V value_{};
};
template <typename K, typename V> entry(K, V) -> entry<K, V>;
} // namespace lookup
