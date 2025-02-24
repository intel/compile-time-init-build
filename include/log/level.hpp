#pragma once

#include <stdx/compiler.hpp>

#include <cstdint>
#include <utility>

namespace logging {
// enum assignment is according to Mipi_Sys-T Severity definition
enum struct level : std::uint8_t {
    MAX = 0,
    FATAL = 1,
    ERROR = 2,
    WARN = 3,
    INFO = 4,
    USER1 = 5,
    USER2 = 6,
    TRACE = 7
};

[[maybe_unused]] constexpr inline struct get_level_t {
    template <typename T>
    CONSTEVAL auto operator()(T &&t) const noexcept(
        noexcept(std::forward<T>(t).query(std::declval<get_level_t>())))
        -> decltype(std::forward<T>(t).query(*this)) {
        return std::forward<T>(t).query(*this);
    }
} get_level;
} // namespace logging
