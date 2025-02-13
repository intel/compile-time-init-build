#pragma once

#include <log/env.hpp>

#include <stdx/compiler.hpp>
#include <stdx/type_traits.hpp>

#include <array>
#include <cstdint>
#include <string_view>
#include <type_traits>
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

template <level L>
[[nodiscard]] constexpr auto to_text() -> std::string_view
    requires(L <= level::TRACE)
{
    using namespace std::string_view_literals;
    constexpr std::array level_text{"MAX"sv,  "FATAL"sv, "ERROR"sv, "WARN"sv,
                                    "INFO"sv, "USER1"sv, "USER2"sv, "TRACE"sv};
    return level_text[stdx::to_underlying(L)];
}

[[maybe_unused]] constexpr inline struct get_level_t {
    template <typename T>
    CONSTEVAL auto operator()(T &&t) const noexcept(
        noexcept(std::forward<T>(t).query(std::declval<get_level_t>())))
        -> decltype(std::forward<T>(t).query(*this)) {
        return std::forward<T>(t).query(*this);
    }
} get_level;
} // namespace logging
