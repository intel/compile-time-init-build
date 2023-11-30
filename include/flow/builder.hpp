#pragma once

#include <cib/builder_meta.hpp>
#include <flow/common.hpp>
#include <flow/graph_builder.hpp>
#include <flow/impl.hpp>

#include <stdx/ct_string.hpp>

namespace flow {
template <stdx::ct_string Name = "">
using builder = graph<Name, graph_builder<impl>>;

template <stdx::ct_string Name = "">
struct service : cib::builder_meta<builder<Name>, FunctionPtr> {};
} // namespace flow
