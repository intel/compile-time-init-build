#pragma once

namespace lookup {
template <typename K, typename V> struct entry {
    using key_type = K;
    using value_type = V;
    key_type key_{};
    value_type value_{};
};
template <typename K, typename V> entry(K, V) -> entry<K, V>;
} // namespace lookup
