#pragma once

#include <cib/builder_meta.hpp>
#include <flow/common.hpp>
#include <flow/graph_builder.hpp>
#include <seq/impl.hpp>

#include <stdx/ct_string.hpp>

namespace seq {
template <stdx::ct_string Name = "">
using builder = flow::graph<Name, flow::graph_builder<impl>>;

template <stdx::ct_string Name = "">
struct service : cib::builder_meta<builder<Name>, flow::FunctionPtr> {};
} // namespace seq
