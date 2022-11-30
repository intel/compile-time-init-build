#include <log/fmt/logger.hpp>

#include <catch2/catch_test_macros.hpp>

#include "testing_logger.hpp"
#include <atomic>

static std::atomic<bool> testing_logger_flag{false};

testing_logger::testing_logger() { testing_logger_flag.store(true); }

testing_logger::~testing_logger() { testing_logger_flag.store(false); }

void *operator new(std::size_t count) {
    if (testing_logger_flag.load()) {
        REQUIRE(false);
    }
    return malloc(count);
}

void operator delete(void *ptr, std::size_t) noexcept { return free(ptr); }

void operator delete(void *ptr) noexcept { return free(ptr); }

TEST_CASE("TRACE doesn't use dynamic memory", "[log]") {
    testing_logger test;
    CIB_TRACE("Hello");
}

TEST_CASE("TRACE doesn't use dynamic memory with larger strings", "[log]") {
    testing_logger test;
    CIB_TRACE("cib trace is not allowed to use dynamic memory");
}
