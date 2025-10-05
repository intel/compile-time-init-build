// include the fmt logger
#include <log/fmt/logger.hpp>

#include <fstream>
#include <iostream>
#include <iterator>

namespace {
// here we use the Myers singleton pattern to get an output iterator to a file
// if we need something more flexible, to use the fmt logger we can
// implement an iterator that can be used with fmt::format_to
auto get_file_output() {
    static auto f = std::ofstream("log.txt");
    return std::ostream_iterator<char>(f);
}
} // namespace

// specialize the logging config variable template to use the fmt logger with
// multiple destinations
template <>
inline auto logging::config<> = logging::fmt::config{
    std::ostream_iterator<char>(std::cout), get_file_output()};

// logs will now use libfmt and go to both cout and the file
auto main() -> int { CIB_INFO("Hello"); }
