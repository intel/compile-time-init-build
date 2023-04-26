#pragma once

#include <cib/builder_meta.hpp>
#include <flow/common.hpp>
#include <flow/graph_builder.hpp>
#include <flow/impl.hpp>
#include <flow/milestone.hpp>

#include <cstddef>

namespace flow {
/**
 * @tparam NodeCapacity
 *      The maximum number of actions and milestones that can be added to a
 *      flow::builder.
 *
 * @tparam EdgeCapacity
 *      The maximum number of dependencies from one action or milestone to
 *      another.
 *
 * @see flow::impl
 * @see flow::graph_builder
 */

template <typename Name = void, std::size_t NodeCapacity = 64,
          std::size_t EdgeCapacity = 16>
struct builder : graph_builder<milestone_base, Name, NodeCapacity, EdgeCapacity,
                               builder<Name, NodeCapacity, EdgeCapacity>> {
    template <typename N, std::size_t Capacity>
    using impl_t = flow::impl<N, Capacity>;
};

/**
 * Extend this to create named flow services.
 *
 * Types that extend flow::meta can be used as unique names with
 * cib::exports and cib::extend.
 *
 * @see cib::exports
 * @see cib::extend
 */
template <typename Name = void, std::size_t NodeCapacity = 64,
          std::size_t EdgeCapacity = 16>
struct service : cib::builder_meta<builder<Name, NodeCapacity, EdgeCapacity>,
                                   FunctionPtr> {};
} // namespace flow
