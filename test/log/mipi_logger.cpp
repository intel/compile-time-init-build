#include <log/catalog/encoder.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/span.hpp>

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
    template <std::size_t N>
    auto operator()(stdx::span<std::uint32_t const, N> pkt) const {
        std::uint32_t const *p = pkt.data();
        CHECK(*p++ == Header);
        (check(*p++, ExpectedArgs), ...);
    }
};
} // namespace

template <>
inline auto logging::config<> = logging::binary::config{
    test_log_version_destination<0b11'000000'1010'1011'1100'1101'0101'0000u>{}};
//                                  3      0    a    b    c    d    5

TEST_CASE("log version", "[mipi_logger]") { CIB_LOG_VERSION(); }
