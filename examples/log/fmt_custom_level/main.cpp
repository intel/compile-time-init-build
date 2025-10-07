// include the fmt logger
#include <log/fmt/logger.hpp>

#include <iostream>
#include <iterator>

// custom level enum
namespace custom {
enum struct level : std::uint8_t { DEBUG = 0, INFO = 1, WARN = 2, ERROR = 3 };
}

// specialize the logging config variable template to use the fmt logger with a
// destination (in this case, std::cout)
// remember: each translation unit that logs must see this same specialization!
template <>
inline auto logging::config<> =
    logging::fmt::config{std::ostream_iterator<char>(std::cout)};

// tell libfmt how to print our custom level
namespace logging {
template <custom::level L>
[[nodiscard]] auto format_as(level_wrapper<L>) -> std::string_view {
    switch (L) {
    case custom::level::DEBUG:
        return "custom_DEBUG";
    case custom::level::INFO:
        return "custom_INFO";
    case custom::level::WARN:
        return "custom_WARN";
    case custom::level::ERROR:
        return "custom_ERROR";
    }
}
} // namespace logging

// set up our own logging macros that use our custom levels
#define CUSTOM_INFO(...)                                                       \
    CIB_LOG_WITH_LEVEL(custom::level::INFO __VA_OPT__(, ) __VA_ARGS__)

// logs will now use our custom levels
auto main() -> int { CUSTOM_INFO("Hello"); }
