#include "catalog_concurrency.hpp"

#include <conc/concurrency.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

template <> inline auto conc::injected_policy<> = test_conc_policy{};

extern int log_calls;
extern std::uint32_t last_header;
extern auto log_zero_args() -> void;
extern auto log_one_ct_arg() -> void;
extern auto log_one_rt_arg() -> void;
extern auto log_two_rt_args() -> void;
extern auto log_rt_enum_arg() -> void;

TEST_CASE("log zero arguments", "[catalog]") {
    test_critical_section::count = 0;
    log_calls = 0;
    log_zero_args();
    CHECK(test_critical_section::count == 2);
    CHECK(log_calls == 1);
    // ID 42 is fixed by stable input
    CHECK(last_header == ((42u << 4u) | 1u));
}

TEST_CASE("log one compile-time argument", "[catalog]") {
    log_calls = 0;
    test_critical_section::count = 0;
    log_one_ct_arg();
    CHECK(test_critical_section::count == 2);
    CHECK(log_calls == 1);
}

TEST_CASE("log one runtime argument", "[catalog]") {
    log_calls = 0;
    test_critical_section::count = 0;
    log_one_rt_arg();
    CHECK(test_critical_section::count == 2);
    CHECK(log_calls == 1);
}

TEST_CASE("log two runtime arguments", "[catalog]") {
    log_calls = 0;
    test_critical_section::count = 0;
    log_two_rt_args();
    CHECK(test_critical_section::count == 2);
    CHECK(log_calls == 1);
}

TEST_CASE("log runtime enum argument", "[catalog]") {
    log_calls = 0;
    test_critical_section::count = 0;
    log_rt_enum_arg();
    CHECK(test_critical_section::count == 2);
    CHECK(log_calls == 1);
}
