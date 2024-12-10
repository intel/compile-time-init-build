#pragma once

#include <flow/common.hpp>
#include <flow/graph_builder.hpp>
#include <seq/impl.hpp>

#include <stdx/ct_string.hpp>

namespace seq {
template <stdx::ct_string Name = "">
using builder = flow::graph<Name, flow::graph_builder<Name, impl>>;

template <stdx::ct_string Name = ""> struct service {
    using builder_t = builder<Name>;
    using interface_t = flow::FunctionPtr;
};
} // namespace seq
