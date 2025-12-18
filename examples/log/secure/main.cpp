#include <log_fmt/logger.hpp>

#include <fstream>
#include <iostream>
#include <iterator>

// declare a type to use for secure logs
struct secure_t;

namespace {
auto get_file_output() {
    static auto f = std::ofstream("log.txt");
    return std::ostream_iterator<char>(f);
}
} // namespace

// specialize the logging config variable template (with no args) to use the
// normal fmt logger
template <>
inline auto logging::config<> =
    logging::fmt::config{std::ostream_iterator<char>(std::cout)};

// specialize the logging config variable template (with the secure arg) to use
// the fmt logger to write to a file
template <>
inline auto logging::config<secure_t> = logging::fmt::config{get_file_output()};

// set up our own secure logging macros that use the secure path
#define SECURE_LOG_WITH_LEVEL(LEVEL, ...)                                      \
    logging::log<stdx::extend_env_t<cib_log_env_t, logging::get_level, LEVEL,  \
                                    logging::get_flavor,                       \
                                    stdx::type_identity<secure_t>{}>>(         \
        __FILE__, __LINE__, STDX_CT_FORMAT(__VA_ARGS__))

#define SECURE_INFO(...)                                                       \
    SECURE_LOG_WITH_LEVEL(logging::level::INFO __VA_OPT__(, ) __VA_ARGS__)

auto main() -> int {
    // ordinary logs go to std::cout
    CIB_INFO("Hello");

    // secure logs go to the file
    SECURE_INFO("Secure hello");
}
