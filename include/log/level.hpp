#pragma once

#include <string_view>
#include <type_traits>

namespace logging {
// enum assignment is according to Mipi_Sys-T Severity definition
enum level {
    MAX = 0,
    FATAL = 1,
    ERROR = 2,
    WARN = 3,
    INFO = 4,
    USER1 = 5,
    USER2 = 6,
    TRACE = 7
};

[[nodiscard]] constexpr auto to_text(level l) -> std::string_view {
    switch (l) {
    case level::TRACE:
        return "TRACE";
    case level::INFO:
        return "INFO";
    case level::WARN:
        return "WARN";
    case level::ERROR:
        return "ERROR";
    case level::FATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

template <level L> struct level_constant : std::integral_constant<level, L> {};
} // namespace logging
