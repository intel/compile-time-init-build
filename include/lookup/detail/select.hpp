#pragma once

#include <cstdint>

namespace lookup::detail {
    template<typename T>
    constexpr inline auto fallback_select(
        std::uint32_t lhs,
        std::uint32_t rhs,
        T first,
        T second
    ) -> T {
        if (lhs == rhs) {
            return first;
        } else {
            return second;
        }
    }

// NOTE: optimized_select is intended to be a branchless select
#if defined(__arc__) or defined(_ARC) or defined(_ARCOMPACT)
    // FIXME: need to handle types larger than 32-bits
    template<typename T>
    static inline auto optimized_select(
        std::uint32_t lhs,
        std::uint32_t rhs,
        T first,
        T second
    ) -> T {
        T result = second;

        asm (
            "   cmp %[lhs], %[rhs]          \n"
            "   mov.eq %[result], %[first]  \n"

            // output operands
            : [result] "+r"(result)

            // input operands
            : [lhs] "r"(lhs)
            , [rhs] "r"(rhs)
            , [first] "r"(first)
        );

        return result;
    }
#else
    template<typename T>
    static inline auto optimized_select(
        std::uint32_t lhs,
        std::uint32_t rhs,
        T first,
        T second
    ) -> T {
        return fallback_select(lhs, rhs, first, second);
    }
#endif

    template<typename T>
    constexpr inline auto select(
        std::uint32_t lhs,
        std::uint32_t rhs,
        T first,
        T second
    ) -> T {
        if (std::is_constant_evaluated()) {
            return fallback_select(lhs, rhs, first, second);

        } else {
            return optimized_select(lhs, rhs, first, second);
        }
    }
}
