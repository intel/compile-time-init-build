#pragma once

#include <flow/builder.hpp>
#include <flow/log.hpp>
#include <seq/impl.hpp>

#include <stdx/ct_string.hpp>

namespace seq {
template <stdx::ct_string Name = "",
          typename LogPolicy = flow::log_policy_t<Name>>
using builder = flow::builder_for<flow::graph_builder<Name, LogPolicy, impl>>;

template <stdx::ct_string Name = "",
          typename LogPolicy = flow::log_policy_t<Name>>
struct service {
    using builder_t = builder<Name, LogPolicy>;
    using interface_t = void (*)();
};
} // namespace seq
