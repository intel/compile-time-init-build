#include "catalog_concurrency.hpp"

#include <conc/concurrency.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

template <> inline auto conc::injected_policy<> = test_conc_policy{};

extern int log_calls;
extern std::uint32_t last_header;
extern auto log_zero_args() -> void;
extern auto log_one_ct_arg() -> void;
extern auto log_one_32bit_rt_arg() -> void;
extern auto log_one_64bit_rt_arg() -> void;
extern auto log_one_formatted_rt_arg() -> void;
extern auto log_two_rt_args() -> void;
extern auto log_rt_enum_arg() -> void;
extern auto log_with_non_default_module() -> void;
extern auto log_with_fixed_module() -> void;
extern auto log_with_fixed_string_id() -> void;
extern auto log_with_fixed_module_id() -> void;

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
    // ID 0 is reserved by stable input
    CHECK(last_header >> 4u != 0);
}

TEST_CASE("log one 32-bit runtime argument", "[catalog]") {
    log_calls = 0;
    test_critical_section::count = 0;
    log_one_32bit_rt_arg();
    CHECK(test_critical_section::count == 2);
    CHECK(log_calls == 1);
}

TEST_CASE("log one 64-bit runtime argument", "[catalog]") {
    log_calls = 0;
    test_critical_section::count = 0;
    log_one_64bit_rt_arg();
    CHECK(test_critical_section::count == 2);
    CHECK(log_calls == 1);
}

TEST_CASE("log one formatted runtime argument", "[catalog]") {
    log_calls = 0;
    test_critical_section::count = 0;
    log_one_formatted_rt_arg();
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

TEST_CASE("log module ids change", "[catalog]") {
    // subtype 1, severity 7, type 3
    std::uint32_t expected_static = (1u << 24u) | (7u << 4u) | 3u;
    log_one_32bit_rt_arg();
    CHECK((last_header & expected_static) == expected_static);

    auto default_header = last_header;
    log_with_non_default_module();
    CHECK((last_header & expected_static) == expected_static);
    CHECK((last_header ^ default_header) == (1u << 16u));
}

TEST_CASE("log with stable module id", "[catalog]") {
    std::uint32_t expected_static = (1u << 24u) | (7u << 4u) | 3u;

    log_with_fixed_module();
    CHECK((last_header & expected_static) == expected_static);
    // module ID 17 is fixed by stable_strings.json
    CHECK((last_header & ~expected_static) == (17u << 16u));
}

TEST_CASE("log with fixed string id", "[catalog]") {
    test_critical_section::count = 0;
    log_calls = 0;
    log_with_fixed_string_id();
    CHECK(test_critical_section::count == 2);
    CHECK(log_calls == 1);
    // string ID 1337 is fixed by environment
    CHECK(last_header == ((1337u << 4u) | 1u));
}

TEST_CASE("log with fixed module id", "[catalog]") {
    std::uint32_t expected_static = (1u << 24u) | (7u << 4u) | 3u;

    log_with_fixed_module_id();
    CHECK((last_header & expected_static) == expected_static);
    // module ID 7 is fixed by environment
    CHECK((last_header & ~expected_static) == (7u << 16u));
}
