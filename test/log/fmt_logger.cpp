#include <log/log.hpp>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <string>

namespace {
auto log_test_override() { CIB_INFO("Hello"); }

std::atomic<bool> allocation_happened{false};
std::string buffer{};
} // namespace

// The override test requires that the specialization here happen after the body
// of log_test_override above.
template <>
auto logging::config<> = logging::fmt::config{std::back_inserter(buffer)};

#ifndef SANITIZER_NEW_DEL
void *operator new(std::size_t count) {
    allocation_happened.store(true);
    return malloc(count);
}

void operator delete(void *ptr) noexcept { return free(ptr); }

// Clang (libc++) requires a previous prototype declaration for sized delete
#ifdef __clang__
void operator delete(void *, std::size_t) noexcept;
#endif
void operator delete(void *ptr, std::size_t) noexcept { return free(ptr); }
#endif

TEST_CASE("logging doesn't use dynamic memory", "[log]") {
    buffer.reserve(100);
    allocation_happened.store(false);
    CIB_TRACE("Hello");
    REQUIRE(not allocation_happened.load());
    REQUIRE(buffer.substr(buffer.size() - std::size("Hello")) == "Hello\n");
}

TEST_CASE("logging behavior can be properly overridden", "[log]") {
    buffer.reserve(100);
    buffer.clear();
    log_test_override();
    REQUIRE(buffer.substr(buffer.size() - std::size("Hello")) == "Hello\n");
}

TEST_CASE("log levels are properly represented", "[log]") {
    {
        std::string level{};
        fmt::format_to(std::back_inserter(level), "{}",
                       logging::level_constant<logging::level::TRACE>{});
        REQUIRE(level == "TRACE");
    }
    {
        std::string level{};
        fmt::format_to(std::back_inserter(level), "{}",
                       logging::level_constant<logging::level::INFO>{});
        REQUIRE(level == "INFO");
    }
    {
        std::string level{};
        fmt::format_to(std::back_inserter(level), "{}",
                       logging::level_constant<logging::level::WARN>{});
        REQUIRE(level == "WARN");
    }
    {
        std::string level{};
        fmt::format_to(std::back_inserter(level), "{}",
                       logging::level_constant<logging::level::ERROR>{});
        REQUIRE(level == "ERROR");
    }
    {
        std::string level{};
        fmt::format_to(std::back_inserter(level), "{}",
                       logging::level_constant<logging::level::FATAL>{});
        REQUIRE(level == "FATAL");
    }
    {
        std::string level{};
        fmt::format_to(std::back_inserter(level), "{}",
                       logging::level_constant<logging::level::MAX>{});
        REQUIRE(level == "UNKNOWN");
    }
}
