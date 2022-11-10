#pragma once

#include <flow/flow.hpp>

namespace interrupt {
using FunctionPtr = std::add_pointer<void()>::type;

template <typename Name = void> using irq_flow = flow::builder<Name, 16, 8>;
} // namespace interrupt
