#pragma once

#include <stdx/span.hpp>

#include <cstddef>
#include <cstdint>

inline int log_calls{};
inline std::uint32_t last_header{};

using log_hook_t = auto (*)(stdx::span<std::uint32_t const>) -> void;
inline log_hook_t log_hook;

struct test_log_destination {
    template <std::size_t N>
    auto operator()(stdx::span<std::uint32_t const, N> pkt) const {
        ++log_calls;
        last_header = pkt[0];
        if (log_hook) {
            log_hook(pkt);
        }
    }
};
