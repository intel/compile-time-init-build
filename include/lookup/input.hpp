#pragma once

#include <lookup/entry.hpp>

#include <array>

namespace lookup {
template <typename K, typename V, V DefaultValue, entry<K, V>... Entries>
struct input {
    using key_type = K;
    using value_type = V;
    constexpr static auto default_value = DefaultValue;
    constexpr static auto size = sizeof...(Entries);
    constexpr static auto entries = std::array<entry<K, V>, size>{Entries...};
};
} // namespace lookup
