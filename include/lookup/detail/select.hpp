#pragma once

#include <cstdint>
#include <type_traits>

namespace lookup::detail {
template <typename T>
constexpr inline auto fallback_select(std::uint32_t lhs, std::uint32_t rhs,
                                      T first, T second) -> T {
    if (lhs == rhs) {
        return first;
    }
    return second;
}

template <typename T>
constexpr inline auto fallback_select_lt(std::uint32_t lhs, std::uint32_t rhs,
                                         T first, T second) -> T {
    if (lhs < rhs) {
        return first;
    }
    return second;
}

// NOTE: optimized_select is intended to be a branchless select
#if defined(__arc__) or defined(_ARC) or defined(_ARCOMPACT)
template <typename T>
static inline auto optimized_select(std::uint32_t lhs, std::uint32_t rhs,
                                    T first, T second) -> T {
    if constexpr (sizeof(T) <= 4) {
        T result = second;

        asm("cmp %[lhs], %[rhs]          \n\t"
            "mov.eq %[result], %[first]  \n\t"

            // output operands
            : [result] "+r,r,r,r,r,r,r,r"(result)

            // input operands
            : [lhs] "r,r,r,r,i,i,i,i"(lhs), [rhs] "r,r,i,i,r,r,i,i"(rhs),
              [first] "r,i,r,i,r,i,r,i"(first));

        return result;
    } else {
        return fallback_select(lhs, rhs, first, second);
    }
}

template <typename T>
static inline auto optimized_select_lt(std::uint32_t lhs, std::uint32_t rhs,
                                       T first, T second) -> T {
    if constexpr (sizeof(T) <= 4) {
        T result = second;

        asm("cmp %[lhs], %[rhs]          \n\t"
            "mov.lo %[result], %[first]  \n\t"

            // output operands
            : [result] "+r"(result)

            // input operands
            : [lhs] "g"(lhs), [rhs] "g"(rhs), [first] "g"(first));

        /*
        // this gives better performance but is probably unsafe
        int is_lo;
        asm volatile (
            "cmp %[lhs], %[rhs]          \n\t"
            // output operands
            : [is_lo] "=@cclo"(is_lo)

            // input operands
            : [lhs] "g"(lhs)
            , [rhs] "g"(rhs)

            // clobbers
            : "cc"
        );

        asm (
            "mov.lo %[result], %[first]  \n\t"

            // output operands
            : [result] "+r"(result)

            // input operands
            : [first] "g"(first)
            , [is_lo] "X"(is_lo)
        );

        */
        return result;

    } else {
        return fallback_select_lt(lhs, rhs, first, second);
    }
}

#else
template <typename T>
static inline auto optimized_select(std::uint32_t lhs, std::uint32_t rhs,
                                    T first, T second) -> T {
    return fallback_select(lhs, rhs, first, second);
}

template <typename T>
static inline auto optimized_select_lt(std::uint32_t lhs, std::uint32_t rhs,
                                       T first, T second) -> T {
    return fallback_select_lt(lhs, rhs, first, second);
}

#endif

template <typename T>
constexpr inline auto select(std::uint32_t lhs, std::uint32_t rhs, T first,
                             T second) -> T {
    if (std::is_constant_evaluated()) {
        return fallback_select(lhs, rhs, first, second);
    }
    return optimized_select(lhs, rhs, first, second);
}

template <typename T>
constexpr inline auto select_lt(std::uint32_t lhs, std::uint32_t rhs, T first,
                                T second) -> T {
    if (std::is_constant_evaluated()) {
        return fallback_select_lt(lhs, rhs, first, second);
    }
    return optimized_select_lt(lhs, rhs, first, second);
}
} // namespace lookup::detail
