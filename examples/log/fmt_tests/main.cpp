// include the fmt logger
#include <log/fmt/logger.hpp>

#include <cassert>
#include <iterator>
#include <string>

// specialize the logging config variable template to use the fmt logger with a
// string destination
std::string log_buffer{};

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(log_buffer)};

// logs will now use libfmt and go into the string where we can test the
// contents and clear the string in test setup code
auto main() -> int {
    CIB_INFO("Hello");
    assert(log_buffer.find("Hello") != std::string::npos);
}
