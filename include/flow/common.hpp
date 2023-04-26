#pragma once

namespace flow {
using FunctionPtr = auto(*)() -> void;

/**
 * While a graph is being built, it is possible for a set of dependencies
 * to be added that result in a circular dependency. build_status is used to
 * enable this to be reported within the constexpr context.
 */
enum class build_status { SUCCESS, HAS_CIRCULAR_DEPENDENCY };
} // namespace flow
