#include <log/fmt/logger.hpp>
#include <log/log.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/panic.hpp>
#include <stdx/tuple.hpp>
#include <stdx/utility.hpp>

#include <catch2/catch_test_macros.hpp>

#include <any>
#include <iterator>
#include <string>
#include <string_view>
#include <tuple>

namespace {
bool panicked{};
std::string_view expected_why{};
std::any expected_args{};

struct injected_handler {
    template <stdx::ct_string Why, typename... Args>
    static auto panic(Args &&...args) noexcept -> void {
        constexpr auto s = std::string_view{Why};
        CAPTURE(s);
        CHECK(s.ends_with(expected_why));
        panicked = true;

        if (expected_args.has_value()) {
            using expected_t = std::tuple<std::decay_t<Args>...>;
            CHECK(std::any_cast<expected_t>(expected_args) ==
                  std::make_tuple(args...));
        }
    }
};

std::string buffer{};

auto reset_test_state() {
    panicked = false;
    expected_why = {};
    expected_args.reset();
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
    using namespace stdx::literals;
    reset_test_state();
    expected_why = "Hello 42";

    CIB_FATAL("{} {}", "Hello", 42);
    CAPTURE(buffer);
    CHECK(buffer.find("Hello 42") != std::string::npos);
    CHECK(panicked);
}

TEST_CASE("CIB_FATAL can format stack arguments (1)", "[log]") {
    reset_test_state();
    expected_why = "Hello {}";
    expected_args = std::make_tuple(stdx::make_tuple(42));

    auto x = 42;
    CIB_FATAL("Hello {}", x);
    CAPTURE(buffer);
    CHECK(buffer.find("Hello 42") != std::string::npos);
    CHECK(panicked);
}

TEST_CASE("CIB_FATAL can format stack arguments (2)", "[log]") {
    reset_test_state();
    expected_why = "Hello {}";
    expected_args =
        std::make_tuple(stdx::make_tuple(std::string_view{"world"}));

    auto x = std::string_view{"world"};
    CIB_FATAL("Hello {}", x);
    CAPTURE(buffer);
    CHECK(buffer.find("Hello world") != std::string::npos);
    CHECK(panicked);
}

TEST_CASE("CIB_FATAL formats compile-time arguments where possible", "[log]") {
    using namespace stdx::literals;
    reset_test_state();
    expected_why = "Hello 17";
    expected_args = std::make_tuple(stdx::make_tuple());

    []<stdx::ct_string S>() {
        CIB_FATAL("{} {}", S, 17);
    }.template operator()<"Hello">();

    CAPTURE(buffer);
    CHECK(buffer.find("Hello 17") != std::string::npos);
    CHECK(panicked);
}

TEST_CASE("CIB_FATAL passes extra arguments to panic", "[log]") {
    reset_test_state();
    expected_why = "Hello {}";
    expected_args = std::make_tuple(stdx::make_tuple(17), 18);

    auto x = 17;
    auto y = 18;
    CIB_FATAL("Hello {}", x, y);
    CAPTURE(buffer);
    CHECK(buffer.find("Hello 17") != std::string::npos);
    CHECK(panicked);
}

TEST_CASE("CIB_ASSERT is equivalent to CIB_FATAL on failure", "[log]") {
    reset_test_state();
    expected_why = "Assertion failure: true == false";

    CIB_ASSERT(true == false);
    CAPTURE(buffer);
    CHECK(buffer.find(expected_why) != std::string::npos);
    CHECK(panicked);
}

TEST_CASE("CIB_ASSERT passes arguments to panic", "[log]") {
    reset_test_state();
    expected_why = "Assertion failure: true == false";
    expected_args = std::make_tuple(stdx::make_tuple(), 17);

    auto x = 17;
    CIB_ASSERT(true == false, x);
    CAPTURE(buffer);
    CHECK(buffer.find(expected_why) != std::string::npos);
    CHECK(panicked);
}
