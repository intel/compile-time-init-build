#pragma once

#include <lookup/entry.hpp>

#include <array>

namespace lookup {
template <typename K, typename V, V default_value_param,
          entry<K, V>... entries_pack>
struct input {
    using key_type = K;
    using value_type = V;
    constexpr static V default_value = default_value_param;
    constexpr static std::array<entry<K, V>, sizeof...(entries_pack)> const
        entries{entries_pack...};
    constexpr static auto size = entries.size();
};
} // namespace lookup
