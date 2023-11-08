#pragma once

#include <stdx/compiler.hpp>

#include <cstdint>

namespace interrupt {
using FunctionPtr = auto (*)() -> void;

enum struct irq_num_t : std::uint32_t {};

// NOLINTNEXTLINE(google-runtime-int)
CONSTEVAL auto operator""_irq(unsigned long long int v) -> irq_num_t {
    return static_cast<irq_num_t>(v);
}
} // namespace interrupt
