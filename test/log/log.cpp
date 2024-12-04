#include <log/fmt/logger.hpp>
#include <log/log.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/panic.hpp>

#include <catch2/catch_test_macros.hpp>

#include <iterator>
#include <string>
#include <string_view>

namespace {
bool panicked{};

struct injected_handler {
    template <stdx::ct_string Why, typename... Ts>
    static auto panic(Ts &&...) noexcept -> void {
        static_assert(std::string_view{Why} == "Hello");
        panicked = true;
    }
};

std::string buffer{};
} // namespace

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(buffer)};

template <> inline auto stdx::panic_handler<> = injected_handler{};

TEST_CASE("CIB_FATAL logs the string", "[log]") {
    CIB_FATAL("Hello");
    CAPTURE(buffer);
    CHECK(buffer.substr(buffer.size() - std::size("Hello")) == "Hello\n");
}

TEST_CASE("CIB_FATAL respects the log module", "[log]") {
    CIB_LOG_MODULE("test");
    CIB_FATAL("Hello");
    CAPTURE(buffer);
    CHECK(buffer.substr(buffer.size() - std::size("FATAL [test]: Hello")) ==
          "FATAL [test]: Hello\n");
}

TEST_CASE("CIB_FATAL calls compile-time panic", "[log]") {
    panicked = false;
    CIB_FATAL("Hello");
    CHECK(panicked);
}

TEST_CASE("CIB_FATAL pre-formats arguments passed to panic", "[log]") {
    panicked = false;
    CIB_FATAL("{}", "Hello"_sc);
    CHECK(panicked);
}
