// include the fmt logger
#include <log_fmt/logger.hpp>

#include <iostream>
#include <iterator>

// specialize the logging config variable template to use the fmt logger with a
// destination (in this case, std::cout)
// remember: each translation unit that logs must see this same specialization!
template <>
inline auto logging::config<> =
    logging::fmt::config{std::ostream_iterator<char>(std::cout)};

// logs will now use libfmt and go to std::cout
auto main() -> int { CIB_INFO("Hello"); }
