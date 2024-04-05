#include <log/catalog/mipi_encoder.hpp>

#include <stdx/ct_string.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

namespace {
struct version_config {
    constexpr static auto build_id = std::uint64_t{0x3abcd5u};
    constexpr static auto version_string = stdx::ct_string{""};
};
} // namespace
template <> inline auto version::config<> = version_config{};

namespace {
template <auto Header, auto... ExpectedArgs>
struct test_log_version_destination {
    template <typename... Args>
    auto log_by_args(std::uint32_t header, Args... args) {
        CHECK(header == Header);
        (check(args, ExpectedArgs), ...);
    }
};
} // namespace

template <>
inline auto logging::config<> = logging::mipi::config{
    test_log_version_destination<0b11'000000'1010'1011'1100'1101'0101'0000u>{}};
//                                  3      0    a    b    c    d    5

TEST_CASE("log version", "[mipi_logger]") { CIB_LOG_VERSION(); }
