#pragma once

#include <concepts>

namespace cib {
template <typename T>
concept builder_meta = requires {
    typename T::builder_t;
    typename T::interface_t;
    { T::uninitialized() } -> std::same_as<typename T::interface_t>;
};

template <builder_meta T> using builder_t = typename T::builder_t;
template <builder_meta T> using interface_t = typename T::interface_t;
} // namespace cib
