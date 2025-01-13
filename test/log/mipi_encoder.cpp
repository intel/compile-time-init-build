#include <conc/concurrency.hpp>
#include <log/catalog/mipi_encoder.hpp>

#include <stdx/concepts.hpp>
#include <stdx/span.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>

namespace {
constexpr string_id test_string_id = 42u;
constexpr module_id test_module_id = 17u;
} // namespace

template <typename StringType> auto catalog() -> string_id {
    return test_string_id;
}

template <typename StringType> auto module() -> module_id {
    return test_module_id;
}

namespace {
struct [[nodiscard]] test_critical_section {
    test_critical_section() { ++count; }
    ~test_critical_section() { ++count; }
    static inline int count = 0;
};

struct test_conc_policy {
    template <typename = void, stdx::invocable F, stdx::predicate... Pred>
        requires(sizeof...(Pred) < 2)
    static inline auto call_in_critical_section(F &&f, Pred &&...pred)
        -> decltype(std::forward<F>(f)()) {
        while (true) {
            [[maybe_unused]] test_critical_section cs{};
            if ((... and pred())) {
                return std::forward<F>(f)();
            }
        }
    }
};

[[maybe_unused]] constexpr auto expected_short32_header() -> std::uint32_t {
    return (static_cast<std::uint32_t>(test_string_id) << 4u) | 0x1u;
}

[[maybe_unused]] constexpr auto
expected_catalog32_header(logging::level level, module_id m) -> std::uint32_t {
    return (0x1u << 24u) | (m << 16u) |
           (static_cast<std::uint32_t>(level) << 4u) | 0x3u;
}

[[maybe_unused]] constexpr auto
expected_msg_header(logging::level level, module_id m,
                    std::size_t sz) -> std::uint32_t {
    return sz > 0 ? expected_catalog32_header(level, m)
                  : expected_short32_header();
}

template <auto ExpectedId> struct test_log_id_destination {
    template <typename Id> static auto log_by_args(Id id) {
        CHECK(id == ((ExpectedId << 4u) | 1u));
    }
};

int num_log_args_calls{};

constexpr auto check = [](auto value, auto expected) {
    CHECK(value == expected);
};

constexpr auto check_at = [](stdx::span<std::uint8_t const> span, auto dw_idx,
                             std::uint32_t expected) {
    auto idx = dw_idx * sizeof(std::uint32_t);
    auto sz = std::min(sizeof(std::uint32_t), span.size() - idx);

    std::uint32_t actual{};
    std::memcpy(&actual, &span[idx], sz);
    CHECK(actual == stdx::to_le(expected));
};

template <std::uint32_t... Expected>
constexpr auto check_buffer = [](stdx::span<std::uint8_t const> data) {
    REQUIRE(data.size() > (sizeof...(Expected) - 1) * sizeof(std::uint32_t));
    auto idx = std::size_t{};
    (check_at(data, idx++, Expected), ...);
};

template <logging::level Level, typename ModuleId, auto... ExpectedArgs>
struct test_log_args_destination {
    template <typename... Args>
    auto log_by_args(std::uint32_t header, Args... args) {
        constexpr auto Header =
            expected_msg_header(Level, test_module_id, sizeof...(ExpectedArgs));
        CHECK(header == Header);
        (check(args, ExpectedArgs), ...);
        ++num_log_args_calls;
    }
};

template <logging::level Level, typename ModuleId, auto... ExpectedArgs>
struct test_log_buf_destination {
    auto log_by_buf(stdx::span<std::uint8_t const> data) const {
        constexpr auto Header =
            expected_msg_header(Level, test_module_id, sizeof...(ExpectedArgs));
        check_buffer<Header, ExpectedArgs...>(data);
        ++num_log_args_calls;
    }
};

template <std::uint32_t Header, std::uint32_t... ExpectedArgs>
struct test_log_version_destination {
    template <typename... Args>
    auto log_by_args(std::uint32_t header, Args... args) {
        CHECK(header == Header);
        (check(args, ExpectedArgs), ...);
        ++num_log_args_calls;
    }

    auto log_by_buf(stdx::span<std::uint8_t const> data) const {
        check_buffer<Header, ExpectedArgs...>(data);
        ++num_log_args_calls;
    }
};
} // namespace

template <> inline auto conc::injected_policy<> = test_conc_policy{};

TEST_CASE("log zero arguments", "[mipi]") {
    test_critical_section::count = 0;
    auto cfg = logging::mipi::config{
        test_log_args_destination<logging::level::TRACE, cib_log_env_t>{}};
    cfg.logger.log_msg<logging::level::TRACE, cib_log_env_t>("Hello"_sc);
    CHECK(test_critical_section::count == 2);
}

TEST_CASE("log one argument", "[mipi]") {
    test_critical_section::count = 0;
    auto cfg = logging::mipi::config{
        test_log_args_destination<logging::level::TRACE, cib_log_env_t, 42u,
                                  17u>{}};
    cfg.logger.log_msg<logging::level::TRACE, cib_log_env_t>(
        format("{}"_sc, 17u));
    CHECK(test_critical_section::count == 2);
}

TEST_CASE("log two arguments", "[mipi]") {
    test_critical_section::count = 0;
    auto cfg = logging::mipi::config{
        test_log_args_destination<logging::level::TRACE, cib_log_env_t, 42u,
                                  17u, 18u>{}};
    cfg.logger.log_msg<logging::level::TRACE, cib_log_env_t>(
        format("{} {}"_sc, 17u, 18u));
    CHECK(test_critical_section::count == 2);
}

TEST_CASE("log more than two arguments", "[mipi]") {
    {
        test_critical_section::count = 0;
        auto cfg = logging::mipi::config{
            test_log_buf_destination<logging::level::TRACE, cib_log_env_t, 42u,
                                     17u, 18u, 19u>{}};
        cfg.logger.log_msg<logging::level::TRACE, cib_log_env_t>(
            format("{} {} {}"_sc, 17u, 18u, 19u));
        CHECK(test_critical_section::count == 2);
    }
    {
        test_critical_section::count = 0;
        auto cfg = logging::mipi::config{
            test_log_buf_destination<logging::level::TRACE, cib_log_env_t, 42u,
                                     17u, 18u, 19u, 20u>{}};
        cfg.logger.log_msg<logging::level::TRACE, cib_log_env_t>(
            format("{} {} {} {}"_sc, 17u, 18u, 19u, 20u));
        CHECK(test_critical_section::count == 2);
    }
}

TEST_CASE("log to multiple destinations", "[mipi]") {
    test_critical_section::count = 0;
    num_log_args_calls = 0;
    auto cfg = logging::mipi::config{
        test_log_args_destination<logging::level::TRACE, cib_log_env_t, 42u,
                                  17u, 18u>{},
        test_log_args_destination<logging::level::TRACE, cib_log_env_t, 42u,
                                  17u, 18u>{}};
    cfg.logger.log_msg<logging::level::TRACE, cib_log_env_t>(
        format("{} {}"_sc, 17u, 18u));
    CHECK(test_critical_section::count == 4);
    CHECK(num_log_args_calls == 2);
}

TEST_CASE("log version information (compact32)", "[mipi]") {
    test_critical_section::count = 0;
    num_log_args_calls = 0;
    auto cfg = logging::mipi::config{test_log_version_destination<
        0b11'000000'1010'1011'1100'1101'0101'0000u>{}};
    //     3      0    a    b    c    d    5
    cfg.logger.log_build<0x3abcd5u>();
    CHECK(test_critical_section::count == 2);
    CHECK(num_log_args_calls == 1);
}

TEST_CASE("log version information (compact64)", "[mipi]") {
    test_critical_section::count = 0;
    num_log_args_calls = 0;
    auto cfg = logging::mipi::config{
        test_log_version_destination<0b11'000001'1010'1011'1100'1101'0101'0000u,
                                     // 3      1    a    b    c    d    5
                                     0b1'0010'0011'0100'0101'0110'00u>{}};
    //                                 1    2    3    4    5    6
    cfg.logger.log_build<0x1234'563a'bcd5u>();
    CHECK(test_critical_section::count == 2);
    CHECK(num_log_args_calls == 1);
}

TEST_CASE("log version information (long with string)", "[mipi]") {
    test_critical_section::count = 0;
    num_log_args_calls = 0;
    auto cfg = logging::mipi::config{
        test_log_version_destination<0b10'0000'0000'0000'0010'0000'0000u,
                                     0x4321'000du, 0x5678'8765u, 0x65'68'1234u,
                                     //                             e  h
                                     0x6f'6c'6cu>{}};
    //                                  o  l  l
    cfg.logger.log_build<0x1234'5678'8765'4321ull, "hello">();
    CHECK(test_critical_section::count == 2);
    CHECK(num_log_args_calls == 1);
}
