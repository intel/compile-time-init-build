#pragma once

#include <cstdint>
#include <type_traits>

namespace lookup::detail {
template <typename K, typename T>
constexpr inline auto fallback_select(K lhs, K rhs, T first, T second) -> T {
    if (lhs == rhs) {
        return first;
    }
    return second;
}

template <typename K, typename T>
constexpr inline auto fallback_select_lt(K lhs, K rhs, T first, T second) -> T {
    if (lhs < rhs) {
        return first;
    }
    return second;
}

// NOTE: optimized_select is intended to be a branchless select
#if defined(__arc__) or defined(_ARC) or defined(_ARCOMPACT)
template <typename K, typename T>
    requires(sizeof(T) <= 4) and (sizeof(K) <= 4)
static inline auto optimized_select(K raw_lhs, K raw_rhs, T first, T second)
    -> T {
    T result = second;
    auto const lhs = static_cast<std::uint32_t>(raw_lhs);
    auto const rhs = static_cast<std::uint32_t>(raw_rhs);

    asm("cmp %[lhs], %[rhs]          \n\t"
        "mov.eq %[result], %[first]  \n\t"

        // output operands
        : [result] "+r,r,r,r,r,r,r,r"(result)

        // input operands
        : [lhs] "r,r,r,r,i,i,i,i"(lhs), [rhs] "r,r,i,i,r,r,i,i"(rhs),
          [first] "r,i,r,i,r,i,r,i"(first));

    return result;
}

template <typename K, typename T>
    requires(sizeof(T) <= 4) and (sizeof(K) <= 4)
static inline auto optimized_select_lt(K raw_lhs, K raw_rhs, T first, T second)
    -> T {
    T result = second;
    auto const lhs = static_cast<std::uint32_t>(raw_lhs);
    auto const rhs = static_cast<std::uint32_t>(raw_rhs);

    asm("cmp %[lhs], %[rhs]          \n\t"
        "mov.lo %[result], %[first]  \n\t"

        // output operands
        : [result] "+r"(result)

        // input operands
        : [lhs] "g"(lhs), [rhs] "g"(rhs), [first] "g"(first));

    return result;
}
#endif

template <typename K, typename T>
static inline auto optimized_select(K lhs, K rhs, T first, T second) -> T {
    return fallback_select(lhs, rhs, first, second);
}

template <typename K, typename T>
static inline auto optimized_select_lt(K lhs, K rhs, T first, T second) -> T {
    return fallback_select_lt(lhs, rhs, first, second);
}

template <typename K, typename T>
constexpr inline auto select(K lhs, K rhs, T first, T second) -> T {
    if (std::is_constant_evaluated()) {
        return fallback_select(lhs, rhs, first, second);
    }
    return optimized_select(lhs, rhs, first, second);
}

template <typename K, typename T>
constexpr inline auto select_lt(K lhs, K rhs, T first, T second) -> T {
    if (std::is_constant_evaluated()) {
        return fallback_select_lt(lhs, rhs, first, second);
    }
    return optimized_select_lt(lhs, rhs, first, second);
}
} // namespace lookup::detail
