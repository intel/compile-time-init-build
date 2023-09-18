#include <conc/concurrency.hpp>
#include <log/catalog/mipi_encoder.hpp>

#include <stdx/concepts.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>

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

constexpr string_id test_string_id = 42u;

[[maybe_unused]] constexpr auto expected_short32_header() -> std::uint32_t {
    return (static_cast<std::uint32_t>(test_string_id) << 4u) | 0x1u;
}

[[maybe_unused]] constexpr auto expected_catalog32_header(logging::level level)
    -> std::uint32_t {
    return (0x1u << 24u) | (static_cast<std::uint32_t>(level) << 4u) | 0x3u;
}

[[maybe_unused]] constexpr auto expected_header(logging::level level, size_t sz)
    -> std::uint32_t {
    return sz > 0 ? expected_catalog32_header(level)
                  : expected_short32_header();
}

template <auto ExpectedId> struct test_log_id_destination {
    template <typename Id> static auto log_by_args(Id id) {
        CHECK(id == ((ExpectedId << 4u) | 1u));
    }
};

int num_log_args_calls{};

template <logging::level Level, auto... ExpectedArgs>
struct test_log_args_destination {
    template <typename... Args>
    auto log_by_args(std::uint32_t header, Args... args) {
        CHECK(header == expected_header(Level, sizeof...(Args)));
        CHECK(((ExpectedArgs == args) and ...));
        ++num_log_args_calls;
    }
};

template <logging::level Level, auto... ExpectedArgs>
struct test_log_buf_destination {
    template <typename... Args>
    auto log_by_buf(std::uint32_t *buf, std::uint32_t size) const {
        CHECK(size == 1 + sizeof...(ExpectedArgs));
        CHECK(*buf++ == expected_header(Level, sizeof...(ExpectedArgs)));
        CHECK(((ExpectedArgs == *buf++) and ...));
    }
};
} // namespace

template <typename StringType> auto catalog() -> string_id {
    return test_string_id;
}

template <> inline auto conc::injected_policy<> = test_conc_policy{};

TEST_CASE("log id", "[mipi]") {
    test_critical_section::count = 0;
    constexpr string_id id{3u};
    auto cfg = logging::mipi::config{test_log_id_destination<id>{}};
    cfg.logger.log_id(id);
    CHECK(test_critical_section::count == 2);
}

TEST_CASE("log zero arguments", "[mipi]") {
    test_critical_section::count = 0;
    auto cfg = logging::mipi::config{
        test_log_args_destination<logging::level::TRACE>{}};
    cfg.logger.log_msg<logging::level::TRACE>("Hello"_sc);
    CHECK(test_critical_section::count == 2);
}

TEST_CASE("log one argument", "[mipi]") {
    test_critical_section::count = 0;
    auto cfg = logging::mipi::config{
        test_log_args_destination<logging::level::TRACE, 42u, 17u>{}};
    cfg.logger.log_msg<logging::level::TRACE>(format("{}"_sc, 17u));
    CHECK(test_critical_section::count == 2);
}

TEST_CASE("log two arguments", "[mipi]") {
    test_critical_section::count = 0;
    auto cfg = logging::mipi::config{
        test_log_args_destination<logging::level::TRACE, 42u, 17u, 18u>{}};
    cfg.logger.log_msg<logging::level::TRACE>(format("{} {}"_sc, 17u, 18u));
    CHECK(test_critical_section::count == 2);
}

TEST_CASE("log more than two arguments", "[mipi]") {
    {
        test_critical_section::count = 0;
        auto cfg = logging::mipi::config{
            test_log_buf_destination<logging::level::TRACE, 42u, 17u, 18u,
                                     19u>{}};
        cfg.logger.log_msg<logging::level::TRACE>(
            format("{} {} {}"_sc, 17u, 18u, 19u));
        CHECK(test_critical_section::count == 2);
    }
    {
        test_critical_section::count = 0;
        auto cfg = logging::mipi::config{
            test_log_buf_destination<logging::level::TRACE, 42u, 17u, 18u, 19u,
                                     20u>{}};
        cfg.logger.log_msg<logging::level::TRACE>(
            format("{} {} {} {}"_sc, 17u, 18u, 19u, 20u));
        CHECK(test_critical_section::count == 2);
    }
}

TEST_CASE("log to multiple destinations", "[mipi]") {
    test_critical_section::count = 0;
    num_log_args_calls = 0;
    auto cfg = logging::mipi::config{
        test_log_args_destination<logging::level::TRACE, 42u, 17u, 18u>{},
        test_log_args_destination<logging::level::TRACE, 42u, 17u, 18u>{}};
    cfg.logger.log_msg<logging::level::TRACE>(format("{} {}"_sc, 17u, 18u));
    CHECK(test_critical_section::count == 4);
    CHECK(num_log_args_calls == 2);
}
