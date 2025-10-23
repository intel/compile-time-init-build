#pragma once

#include <flow/common.hpp>
#include <flow/graph_builder.hpp>
#include <flow/log.hpp>
#include <seq/impl.hpp>

#include <stdx/ct_string.hpp>

namespace seq {
template <stdx::ct_string Name = "",
          typename LogPolicy = flow::log_policy_t<Name>>
using builder =
    flow::graph<Name, LogPolicy, flow::graph_builder<Name, impl, LogPolicy>>;

template <stdx::ct_string Name = "",
          typename LogPolicy = flow::log_policy_t<Name>>
struct service {
    using builder_t = builder<Name, LogPolicy>;
    using interface_t = flow::FunctionPtr;
};
} // namespace seq
