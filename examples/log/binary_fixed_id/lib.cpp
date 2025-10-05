#include "logger.hpp"

auto lib_func() -> void {
    // For this specific log call, fix the string ID to 1337
    CIB_WITH_LOG_ENV(logging::get_string_id, 1337) { CIB_INFO("Hello"); }
    // The same technique can be used to fix any particular attribute query for
    // a single call.
}
