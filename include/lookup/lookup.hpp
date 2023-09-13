#pragma once
#include <lookup/strategy/arc_cpu.hpp>

namespace lookup {
// TODO: need a good way to make this extendable
template <typename InputValues> [[nodiscard]] consteval static auto make() {
    return lookup::strategy::arc_cpu::make<InputValues>();
}
} // namespace lookup
