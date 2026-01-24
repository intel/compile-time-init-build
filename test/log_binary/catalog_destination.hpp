#pragma once

#include <stdx/span.hpp>

#include <cstddef>
#include <cstdint>

inline int log_calls{};
inline std::uint32_t last_header{};

struct test_log_destination {
    template <std::size_t N>
    auto operator()(stdx::span<std::uint32_t const, N> pkt) const {
        ++log_calls;
        last_header = pkt[0];
    }
};
