#include <log/catalog/mipi_encoder.hpp>
#include <log/fmt/logger.hpp>
#include <log/level.hpp>
#include <sc/format.hpp>

#include <stdx/ct_conversions.hpp>
#include <stdx/ct_string.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <iterator>
#include <string>
#include <type_traits>

namespace {
enum struct custom_level { THE_ONE_LEVEL = 5 };

std::string buffer{};
} // namespace

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(buffer)};

template <custom_level L>
struct fmt::formatter<std::integral_constant<custom_level, L>> {
    constexpr static auto parse(format_parse_context &ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(std::integral_constant<custom_level, L>,
                FormatContext &ctx) const {
        return ::fmt::format_to(ctx.out(), stdx::enum_as_string<L>());
    }
};

TEST_CASE("fmt logger works with custom level", "[level]") {
    CIB_LOG_ENV(logging::get_level, custom_level::THE_ONE_LEVEL);
    buffer.clear();
    CIB_LOG(logging::default_flavor_t, "Hello");
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
    auto cfg = logging::mipi::config{test_destination{}};
    cfg.logger.log_msg<cib_log_env_t>(sc::format("Hello {} {}"_sc, 17, 42));
    CHECK(log_calls == 1);
}
