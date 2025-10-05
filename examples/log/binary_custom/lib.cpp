#include "logger.hpp"

namespace lib {

// Provide an environment for this scope that uses our builder.
CIB_LOG_ENV(logging::binary::get_builder, custom::builder{});

auto lib_func() -> void { CIB_INFO("Hello"); }
} // namespace lib
