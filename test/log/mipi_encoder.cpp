#include <log/catalog/mipi_encoder.hpp>

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

auto expected_header(log_level level) -> std::uint32_t {
    return (0x1u << 24u) | (static_cast<std::uint32_t>(level) << 4u) | 0x3u;
}

template <auto ExpectedId> struct test_log_id_destination {
    template <typename Id> static auto log_by_args(Id id) {
        REQUIRE(id == ((ExpectedId << 4u) | 1u));
    }
};

template <log_level Level, auto... ExpectedArgs>
struct test_log_args_destination {
    template <typename... Args>
    static auto log_by_args(std::uint32_t header, Args... args) {
        REQUIRE(header == expected_header(Level));
        REQUIRE(((ExpectedArgs == args) and ...));
    }
};

template <log_level Level, auto... ExpectedArgs>
struct test_log_buf_destination {
    template <typename... Args>
    static auto log_by_buf(std::uint32_t *buf, std::uint32_t size) {
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
    using test_logger =
        mipi::mipi_encoder<test_critical_section, test_log_id_destination<id>>;
    test_logger::log_id(id);
    REQUIRE(test_critical_section::count == 2);
}

TEST_CASE("log one argument", "[mipi]") {
    test_critical_section::count = 0;
    using test_logger = mipi::mipi_encoder<
        test_critical_section,
        test_log_args_destination<log_level::TRACE, 42u, 17u>>;
    test_logger::log_impl<log_level::TRACE>(format("{}"_sc, 17u));
    REQUIRE(test_critical_section::count == 2);
}

TEST_CASE("log two arguments", "[mipi]") {
    test_critical_section::count = 0;
    using test_logger = mipi::mipi_encoder<
        test_critical_section,
        test_log_args_destination<log_level::TRACE, 42u, 17u, 18u>>;
    test_logger::log_impl<log_level::TRACE>(format("{} {}"_sc, 17u, 18u));
    REQUIRE(test_critical_section::count == 2);
}

TEST_CASE("log more than two arguments", "[mipi]") {
    {
        test_critical_section::count = 0;
        using test_logger = mipi::mipi_encoder<
            test_critical_section,
            test_log_buf_destination<log_level::TRACE, 42u, 17u, 18u, 19u>>;
        test_logger::log_impl<log_level::TRACE>(
            format("{} {} {}"_sc, 17u, 18u, 19u));
        REQUIRE(test_critical_section::count == 2);
    }
    {
        test_critical_section::count = 0;
        using test_logger =
            mipi::mipi_encoder<test_critical_section,
                               test_log_buf_destination<log_level::TRACE, 42u,
                                                        17u, 18u, 19u, 20u>>;
        test_logger::log_impl<log_level::TRACE>(
            format("{} {} {} {}"_sc, 17u, 18u, 19u, 20u));
        REQUIRE(test_critical_section::count == 2);
    }
}

TEST_CASE("log to multiple destinations", "[mipi]") {
    test_critical_section::count = 0;
    using test_logger = mipi::mipi_encoder<
        test_critical_section,
        test_log_args_destination<log_level::TRACE, 42u, 17u, 18u>,
        test_log_args_destination<log_level::TRACE, 42u, 17u, 18u>>;
    test_logger::log_impl<log_level::TRACE>(format("{} {}"_sc, 17u, 18u));
    REQUIRE(test_critical_section::count == 2);
}
