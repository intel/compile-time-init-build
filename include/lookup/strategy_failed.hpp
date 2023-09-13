#pragma once

#include <stdx/compiler.hpp>

#include <type_traits>

namespace lookup {
struct strategy_failed_t {};

template <typename T>
[[nodiscard]] CONSTEVAL auto strategy_failed(T const &) -> bool {
    return std::is_same_v<T, strategy_failed_t>;
}
} // namespace lookup
