#include "catalog_concurrency.hpp"

#include <conc/concurrency.hpp>

#include <catch2/catch_test_macros.hpp>

template <> inline auto conc::injected_policy<> = test_conc_policy{};

extern int log_calls;
extern auto log_zero_args() -> void;
extern auto log_one_ct_arg() -> void;
extern auto log_one_rt_arg() -> void;

TEST_CASE("log zero arguments", "[catalog]") {
    test_critical_section::count = 0;
    log_calls = 0;
    log_zero_args();
    CHECK(test_critical_section::count == 2);
    CHECK(log_calls == 1);
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
