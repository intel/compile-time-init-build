#include <log/catalog/encoder.hpp>
#include <log/fmt/logger.hpp>
#include <log/level.hpp>

#include <stdx/ct_conversions.hpp>
#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <type_traits>

namespace {
enum struct custom_level { THE_ONE_LEVEL = 5 };

std::string buffer{};
} // namespace

namespace logging {
template <custom_level L>
[[nodiscard]] constexpr auto format_as(level_wrapper<L>) -> std::string_view {
    static_assert(L == custom_level::THE_ONE_LEVEL);
    return "THE_ONE_LEVEL";
}
} // namespace logging

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(buffer)};

TEST_CASE("fmt logger works with custom level", "[level]") {
    CIB_LOG_ENV(logging::get_level, custom_level::THE_ONE_LEVEL);
    buffer.clear();
    CIB_LOG("Hello");
    CAPTURE(buffer);
    CHECK(buffer.find("THE_ONE_LEVEL [default]:") != std::string::npos);
}

template <typename> auto catalog() -> string_id { return 0xdeadbeef; }
template <typename> auto module() -> module_id { return 0x5a; }

namespace {
int log_calls{};

struct test_destination {
    auto log_by_args(std::uint32_t header, auto id, auto &&...) {
        CHECK(header == 0x01'5a'00'53);
        CHECK(id == 0xdeadbeef);
        ++log_calls;
    }
};
} // namespace

TEST_CASE("mipi logger works with custom level", "[level]") {
    log_calls = 0;
    CIB_LOG_ENV(logging::get_level, custom_level::THE_ONE_LEVEL);
    auto cfg = logging::binary::config{test_destination{}};
    cfg.logger.log_msg<cib_log_env_t>(stdx::ct_format<"Hello {} {}">(17, 42));
    CHECK(log_calls == 1);
}

namespace {
constexpr auto MY_LEVEL = logging::level::USER1;
}
namespace logging {
template <> constexpr std::string_view level_text<MY_LEVEL> = "MY_LEVEL";
}

TEST_CASE("mipi logger USER level can be renamed", "[level]") {
    CIB_LOG_ENV(logging::get_level, MY_LEVEL);
    buffer.clear();
    CIB_LOG("Hello");
    CAPTURE(buffer);
    CHECK(buffer.find("MY_LEVEL [default]:") != std::string::npos);
}
