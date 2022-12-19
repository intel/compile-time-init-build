#include <log/log.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
struct test_critical_section {
    test_critical_section() {
        REQUIRE(count % 2 == 0);
        ++count;
    }
    ~test_critical_section() {
        REQUIRE(count % 2 == 1);
        ++count;
    }
    static int count;
};
int test_critical_section::count = 0;

[[maybe_unused]] constexpr auto expected_header(logging::level level)
    -> std::uint32_t {
    return (0x1u << 24u) | (static_cast<std::uint32_t>(level) << 4u) | 0x3u;
}

template <auto ExpectedId> struct test_log_id_destination {
    template <typename Id> static auto log_by_args(Id id) {
        REQUIRE(id == ((ExpectedId << 4u) | 1u));
    }
};

int num_log_args_calls{};

template <logging::level Level, auto... ExpectedArgs>
struct test_log_args_destination {
    template <typename... Args>
    auto log_by_args(std::uint32_t header, Args... args) {
        REQUIRE(header == expected_header(Level));
        REQUIRE(((ExpectedArgs == args) and ...));
        ++num_log_args_calls;
    }
};

template <logging::level Level, auto... ExpectedArgs>
struct test_log_buf_destination {
    template <typename... Args>
    auto log_by_buf(std::uint32_t *buf, std::uint32_t size) const {
        REQUIRE(size == 1 + sizeof...(ExpectedArgs));
        REQUIRE(*buf++ == expected_header(Level));
        REQUIRE(((ExpectedArgs == *buf++) and ...));
    }
};
} // namespace

template <typename StringType> auto catalog() -> string_id { return 42u; }

TEST_CASE("log id", "[mipi]") {
    test_critical_section::count = 0;
    constexpr string_id id{3u};
    auto cfg = logging::mipi::under<test_critical_section>::config{
        test_log_id_destination<id>{}};
    cfg.logger.log_id(id);
    REQUIRE(test_critical_section::count == 2);
}

TEST_CASE("log one argument", "[mipi]") {
    test_critical_section::count = 0;
    auto cfg = logging::mipi::under<test_critical_section>::config{
        test_log_args_destination<logging::level::TRACE, 42u, 17u>{}};
    cfg.logger.log_msg<logging::level::TRACE>(format("{}"_sc, 17u));
    REQUIRE(test_critical_section::count == 2);
}

TEST_CASE("log two arguments", "[mipi]") {
    test_critical_section::count = 0;
    auto cfg = logging::mipi::under<test_critical_section>::config{
        test_log_args_destination<logging::level::TRACE, 42u, 17u, 18u>{}};
    cfg.logger.log_msg<logging::level::TRACE>(format("{} {}"_sc, 17u, 18u));
    REQUIRE(test_critical_section::count == 2);
}

TEST_CASE("log more than two arguments", "[mipi]") {
    {
        test_critical_section::count = 0;
        auto cfg = logging::mipi::under<test_critical_section>::config{
            test_log_buf_destination<logging::level::TRACE, 42u, 17u, 18u,
                                     19u>{}};
        cfg.logger.log_msg<logging::level::TRACE>(
            format("{} {} {}"_sc, 17u, 18u, 19u));
        REQUIRE(test_critical_section::count == 2);
    }
    {
        test_critical_section::count = 0;
        auto cfg = logging::mipi::under<test_critical_section>::config{
            test_log_buf_destination<logging::level::TRACE, 42u, 17u, 18u, 19u,
                                     20u>{}};
        cfg.logger.log_msg<logging::level::TRACE>(
            format("{} {} {} {}"_sc, 17u, 18u, 19u, 20u));
        REQUIRE(test_critical_section::count == 2);
    }
}

TEST_CASE("log to multiple destinations", "[mipi]") {
    test_critical_section::count = 0;
    num_log_args_calls = 0;
    auto cfg = logging::mipi::under<test_critical_section>::config{
        test_log_args_destination<logging::level::TRACE, 42u, 17u, 18u>{},
        test_log_args_destination<logging::level::TRACE, 42u, 17u, 18u>{}};
    cfg.logger.log_msg<logging::level::TRACE>(format("{} {}"_sc, 17u, 18u));
    REQUIRE(test_critical_section::count == 2);
    REQUIRE(num_log_args_calls == 2);
}
