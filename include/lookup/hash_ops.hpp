#pragma once

namespace lookup {
template <typename... Ts> struct ops {
    [[nodiscard]] constexpr static inline auto calc(auto x) {
        return ((x = Ts::calc(x)), ...);
    }
};

template <auto imm> struct subl_op {
    [[nodiscard]] constexpr static inline auto calc(auto x) {
        return x - (x << imm);
    }
};

template <auto imm> struct addl_op {
    [[nodiscard]] constexpr static inline auto calc(auto x) {
        return x + (x << imm);
    }
};

template <auto imm> struct xorl_op {
    [[nodiscard]] constexpr static inline auto calc(auto x) {
        return x ^ (x << imm);
    }
};

template <auto imm> struct xorr_op {
    [[nodiscard]] constexpr static inline auto calc(auto x) {
        return x ^ (x >> imm);
    }
};

template <auto imm> struct mul_op {
    [[nodiscard]] constexpr static inline auto calc(auto x) { return x * imm; }
};

struct id_op {
    [[nodiscard]] constexpr static inline auto calc(auto x) { return x; }
};
} // namespace lookup
