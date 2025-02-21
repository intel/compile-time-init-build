#include <log/fmt/logger.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/utility.hpp>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <iostream>
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
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(buffer)};

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

TEST_CASE("logging doesn't use dynamic memory", "[fmt_logger]") {
    buffer.reserve(100);
    buffer.clear();
    allocation_happened.store(false);
    CIB_TRACE("Hello");
    CHECK(not allocation_happened.load());
    CAPTURE(buffer);
    CHECK(buffer.substr(buffer.size() - std::size("Hello")) == "Hello\n");
}

TEST_CASE("logging behavior can be properly overridden", "[fmt_logger]") {
    buffer.reserve(100);
    buffer.clear();
    log_test_override();
    CAPTURE(buffer);
    CHECK(buffer.substr(buffer.size() - std::size("Hello")) == "Hello\n");
}

TEST_CASE("log levels are properly represented", "[fmt_logger]") {
    {
        std::string level{};
        fmt::format_to(std::back_inserter(level), "{}",
                       logging::level_wrapper<logging::level::TRACE>{});
        CHECK(level == "TRACE");
    }
    {
        std::string level{};
        fmt::format_to(std::back_inserter(level), "{}",
                       logging::level_wrapper<logging::level::INFO>{});
        CHECK(level == "INFO");
    }
    {
        std::string level{};
        fmt::format_to(std::back_inserter(level), "{}",
                       logging::level_wrapper<logging::level::WARN>{});
        CHECK(level == "WARN");
    }
    {
        std::string level{};
        fmt::format_to(std::back_inserter(level), "{}",
                       logging::level_wrapper<logging::level::ERROR>{});
        CHECK(level == "ERROR");
    }
    {
        std::string level{};
        fmt::format_to(std::back_inserter(level), "{}",
                       logging::level_wrapper<logging::level::FATAL>{});
        CHECK(level == "FATAL");
    }
    {
        std::string level{};
        fmt::format_to(std::back_inserter(level), "{}",
                       logging::level_wrapper<logging::level::MAX>{});
        CHECK(level == "MAX");
    }
    {
        std::string level{};
        fmt::format_to(std::back_inserter(level), "{}",
                       logging::level_wrapper<logging::level::USER1>{});
        CHECK(level == "USER1");
    }
    {
        std::string level{};
        fmt::format_to(std::back_inserter(level), "{}",
                       logging::level_wrapper<logging::level::USER2>{});
        CHECK(level == "USER2");
    }
}

TEST_CASE("log level is reported", "[fmt_logger]") {
    buffer.clear();
    CIB_TRACE("Hello");
    CAPTURE(buffer);
    CHECK(buffer.find("TRACE") != std::string::npos);
}

TEST_CASE("log module id is reported", "[fmt_logger]") {
    buffer.clear();
    CIB_TRACE("Hello");
    CAPTURE(buffer);
    CHECK(buffer.find("[default]") != std::string::npos);

    {
        CIB_LOG_MODULE("test");
        buffer.clear();
        CIB_TRACE("Hello");
        CAPTURE(buffer);
        CHECK(buffer.find("[test]") != std::string::npos);
    }
}

TEST_CASE("logging can use std::cout", "[fmt_logger]") {
    [[maybe_unused]] auto cfg =
        logging::fmt::config{std::ostream_iterator<char>{std::cout}};
}

namespace {
struct version_config {
    constexpr static auto build_id = std::uint64_t{1234};
    constexpr static auto version_string = stdx::ct_string{"test version"};
};
} // namespace
template <> inline auto version::config<> = version_config{};

TEST_CASE("log version", "[fmt_logger]") {
    buffer.clear();
    CIB_LOG_VERSION();
    CAPTURE(buffer);
    CHECK(buffer.find("MAX [default]: Version: 1234 (test version)") !=
          std::string::npos);
}

namespace {
struct secure_t;
std::string secure_buffer{};
} // namespace

template <>
inline auto logging::config<secure_t> =
    logging::fmt::config{std::back_inserter(secure_buffer)};

#define SECURE_TRACE(MSG, ...)                                                 \
    logging::log<logging::extend_env_t<                                        \
        cib_log_env_t, logging::get_level, logging::level::TRACE,              \
        logging::get_flavor, stdx::type_identity<secure_t>{}>>(                \
        __FILE__, __LINE__, sc::format(MSG##_sc __VA_OPT__(, ) __VA_ARGS__))

TEST_CASE("logging can be flavored", "[fmt_logger]") {
    buffer.clear();
    secure_buffer.clear();
    SECURE_TRACE("Hello");
    CAPTURE(secure_buffer);
    CHECK(secure_buffer.substr(secure_buffer.size() - std::size("Hello")) ==
          "Hello\n");
    CHECK(buffer.empty());
}

#undef SECURE_TRACE

TEST_CASE("log version can be flavored", "[fmt_logger]") {
    buffer.clear();
    secure_buffer.clear();
    CIB_LOG_V(secure_t);
    CAPTURE(secure_buffer);
    CHECK(secure_buffer.find("MAX [default]: Version: 1234 (test version)") !=
          std::string::npos);
    CHECK(buffer.empty());
}
