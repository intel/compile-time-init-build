#pragma once

#include <cib/builder_meta.hpp>
#include <flow/common.hpp>
#include <flow/graph_builder.hpp>
#include <seq/impl.hpp>
#include <seq/step.hpp>

#include <stdx/ct_string.hpp>

#include <cstddef>

namespace seq {
/**
 * @tparam NodeCapacity
 *      The maximum number of actions and milestones that can be added to a
 *      seq::builder.
 *
 * @tparam EdgeCapacity
 *      The maximum number of dependencies from one action or milestone to
 *      another.
 *
 * @see seq::impl
 * @see flow::graph_builder
 */

template <stdx::ct_string Name = "", std::size_t NodeCapacity = 64,
          std::size_t EdgeCapacity = 16>
using builder =
    flow::graph_builder<step_base, impl, Name, NodeCapacity, EdgeCapacity>;

/**
 * Extend this to create named seq services.
 *
 * Types that extend seq::meta can be used as unique names with
 * cib::exports and cib::extend.
 *
 * @see cib::exports
 * @see cib::extend
 */
template <stdx::ct_string Name = "", std::size_t NodeCapacity = 64,
          std::size_t EdgeCapacity = 16>
struct service : cib::builder_meta<builder<Name, NodeCapacity, EdgeCapacity>,
                                   flow::FunctionPtr> {};
} // namespace seq
