#pragma once

#include <nexus/built.hpp>

namespace flow {
/**
 * Run the flow given by 'Tag'.
 *
 * @tparam Tag Type of the flow to be ran. This is the name of the flow::builder
 * used to declare and build the flow.
 */
template <typename Tag> auto &run = cib::service<Tag>;
} // namespace flow
