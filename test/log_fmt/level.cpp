#include <log/level.hpp>
#include <log_fmt/logger.hpp>

#include <catch2/catch_test_macros.hpp>

#include <iterator>
#include <string>
#include <string_view>

namespace {
enum struct custom_level { THE_ONE_LEVEL = 5 };

std::string buffer{};
} // namespace

namespace logging {
template <custom_level L>
[[nodiscard]] auto format_as(level_wrapper<L>) -> std::string_view {
    STATIC_REQUIRE(L == custom_level::THE_ONE_LEVEL);
    return "THE_ONE_LEVEL";
}
} // namespace logging

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(buffer)};

TEST_CASE("fmt logger works with custom level", "[fmt_level]") {
    CIB_LOG_ENV(logging::get_level, custom_level::THE_ONE_LEVEL);
    buffer.clear();
    CIB_LOG("Hello");
    CAPTURE(buffer);
    CHECK(buffer.find("THE_ONE_LEVEL [default]:") != std::string::npos);
}

namespace {
constexpr auto MY_LEVEL = logging::level::USER1;
}
namespace logging {
template <> constexpr std::string_view level_text<MY_LEVEL> = "MY_LEVEL";
}

TEST_CASE("fmt logger USER level can be renamed", "[fmt_level]") {
    CIB_LOG_ENV(logging::get_level, MY_LEVEL);
    buffer.clear();
    CIB_LOG("Hello");
    CAPTURE(buffer);
    CHECK(buffer.find("MY_LEVEL [default]:") != std::string::npos);
}
