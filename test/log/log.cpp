#include <log/fmt/logger.hpp>
#include <log/log.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/panic.hpp>

#include <catch2/catch_test_macros.hpp>

#include <iterator>
#include <optional>
#include <string>
#include <string_view>

namespace {
bool panicked{};
std::string_view expected_why{};
std::optional<int> expected_arg{};

struct injected_handler {
    template <stdx::ct_string Why, typename... Args>
    static auto panic(Args &&...args) noexcept -> void {
        constexpr auto s = std::string_view{Why};
        CAPTURE(s);
        CHECK(s.ends_with(expected_why));
        panicked = true;
        if (expected_arg) {
            CHECK(sizeof...(Args) == 1);
            if constexpr (sizeof...(Args) == 1) {
                CHECK(*expected_arg == (args, ...));
            }
        }
    }
};

std::string buffer{};

auto reset_test_state() {
    panicked = false;
    expected_why = {};
    expected_arg.reset();
    buffer.clear();
}
} // namespace

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(buffer)};

template <> inline auto stdx::panic_handler<> = injected_handler{};

TEST_CASE("CIB_FATAL logs the string", "[log]") {
    reset_test_state();

    CIB_FATAL("Hello");
    CAPTURE(buffer);
    CHECK(buffer.find("Hello") != std::string::npos);
}

TEST_CASE("CIB_FATAL respects the log module", "[log]") {
    reset_test_state();

    CIB_LOG_MODULE("test");
    CIB_FATAL("Hello");
    CAPTURE(buffer);
    CHECK(buffer.find("FATAL [test]: Hello") != std::string::npos);
}

TEST_CASE("CIB_FATAL calls compile-time panic", "[log]") {
    reset_test_state();
    expected_why = "Hello";

    CIB_FATAL("Hello");
    CHECK(panicked);
}

TEST_CASE("CIB_FATAL pre-formats arguments passed to panic", "[log]") {
    reset_test_state();
    expected_why = "Hello 42";

    CIB_FATAL("{} {}", "Hello"_sc, sc::int_<42>);
    CAPTURE(buffer);
    CHECK(buffer.find("Hello 42") != std::string::npos);
    CHECK(panicked);
}

TEST_CASE("CIB_FATAL can format stack arguments", "[log]") {
    reset_test_state();
    expected_why = "Hello {}";
    expected_arg = 42;

    auto x = 42;
    CIB_FATAL("Hello {}", x);
    CAPTURE(buffer);
    CHECK(buffer.find("Hello 42") != std::string::npos);
    CHECK(panicked);
}
