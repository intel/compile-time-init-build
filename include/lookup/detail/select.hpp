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

    template<typename T>
    constexpr inline auto fallback_select_lt(
        std::uint32_t lhs,
        std::uint32_t rhs,
        T first,
        T second
    ) -> T {
        if (lhs < rhs) {
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
            "cmp %[lhs], %[rhs]          \n\t"
            "mov.eq %[result], %[first]  \n\t"

            // output operands
            : [result] "+r"(result)

            // input operands
            : [lhs] "g"(lhs)
            , [rhs] "g"(rhs)
            , [first] "g"(first)
        );

        return result;
    }

    template<typename T>
    static inline auto optimized_select_lt(
        std::uint32_t lhs,
        std::uint32_t rhs,
        T first,
        T second
    ) -> T {
        T result = second;

        asm (
            "cmp %[lhs], %[rhs]          \n\t"
            "mov.lo %[result], %[first]  \n\t"

            // output operands
            : [result] "+r"(result)

            // input operands
            : [lhs] "g"(lhs)
            , [rhs] "g"(rhs)
            , [first] "g"(first)
        );

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

    template<typename T>
    static inline auto optimized_select_lt(
        std::uint32_t lhs,
        std::uint32_t rhs,
        T first,
        T second
    ) -> T {
        return fallback_select_lt(lhs, rhs, first, second);
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


    template<typename T>
    constexpr inline auto select_lt(
        std::uint32_t lhs,
        std::uint32_t rhs,
        T first,
        T second
    ) -> T {
        if (std::is_constant_evaluated()) {
            return fallback_select_lt(lhs, rhs, first, second);

        } else {
            return optimized_select_lt(lhs, rhs, first, second);
        }
    }

}
