#pragma once

#include <cib/built.hpp>

namespace flow {
namespace detail {
using FunctionPtr = auto (*)() -> void;
}

/**
 * Run the flow given by 'Tag'.
 *
 * @tparam Tag Type of the flow to be ran. This is the name of the flow::builder
 * used to declare and build the flow.
 */
template <typename Tag> detail::FunctionPtr &run = cib::service<Tag>;
} // namespace flow
