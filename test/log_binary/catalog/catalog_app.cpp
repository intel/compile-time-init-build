#include "catalog_concurrency.hpp"
#include "catalog_destination.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <fstream>

extern auto log_zero_args() -> void;
extern auto log_one_ct_arg() -> void;
extern auto log_with_fixed_string_id() -> void;
extern auto log_with_fixed_unsigned_string_id() -> void;

extern auto log_one_32bit_rt_arg() -> void;
extern auto log_one_64bit_rt_arg() -> void;
extern auto log_one_formatted_rt_arg() -> void;
extern auto log_two_rt_args() -> void;

auto log_rt_scoped_enum_arg() -> void;
auto log_rt_unscoped_enum_arg() -> void;
extern auto log_rt_auto_scoped_enum_arg() -> void;

extern auto log_rt_float_arg() -> void;
extern auto log_rt_double_arg() -> void;

extern auto log_with_default_module() -> void;
extern auto log_with_non_default_module() -> void;
extern auto log_with_fixed_module() -> void;
extern auto log_with_fixed_module_id() -> void;
extern auto log_with_fixed_unsigned_module_id() -> void;

std::ofstream log_file{};

struct log_file_writer {
    log_file_writer() {
        log_file.open("catalog_test.bin", std::ios::out | std::ios::binary);
        log_hook = [](stdx::span<std::uint32_t const> data) {
            using A = std::array<char, sizeof(std::uint32_t)>;
            for (auto i : data) {
                auto a = stdx::bit_cast<A>(i);
                log_file.write(a.data(), a.size());
            }
        };
    }

    ~log_file_writer() { log_file.close(); }
};

struct fixture {
    log_file_writer l;
};

TEST_CASE_PERSISTENT_FIXTURE(fixture, "catalog tests") {

    // short messages

    SECTION("log zero arguments") {
        test_critical_section::count = 0;
        log_calls = 0;
        log_zero_args();
        CHECK(test_critical_section::count == 2);
        CHECK(log_calls == 1);
        // ID 42 is fixed by stable input
        CHECK(last_header == ((42u << 4u) | 1u));
    }

    SECTION("log one compile-time argument") {
        log_calls = 0;
        test_critical_section::count = 0;
        log_one_ct_arg();
        CHECK(test_critical_section::count == 2);
        CHECK(log_calls == 1);
        // ID 0 is reserved by stable input
        CHECK(last_header >> 4u != 0);
    }

    SECTION("log with fixed string id") {
        test_critical_section::count = 0;
        log_calls = 0;
        log_with_fixed_string_id();
        CHECK(test_critical_section::count == 2);
        CHECK(log_calls == 1);
        // string ID 1337 is fixed by environment
        CHECK(last_header == ((1337u << 4u) | 1u));
    }

    SECTION("log with fixed unsigned string id") {
        test_critical_section::count = 0;
        log_calls = 0;
        log_with_fixed_unsigned_string_id();
        CHECK(test_critical_section::count == 2);
        CHECK(log_calls == 1);
        // string ID 1338 is fixed by environment
        CHECK(last_header == ((1338u << 4u) | 1u));
    }

    // integral runtime arguments

    SECTION("log one 32-bit runtime argument") {
        log_calls = 0;
        test_critical_section::count = 0;
        log_one_32bit_rt_arg();
        CHECK(test_critical_section::count == 2);
        CHECK(log_calls == 1);
    }

    SECTION("log one 64-bit runtime argument") {
        log_calls = 0;
        test_critical_section::count = 0;
        log_one_64bit_rt_arg();
        CHECK(test_critical_section::count == 2);
        CHECK(log_calls == 1);
    }

    SECTION("log one formatted runtime argument") {
        log_calls = 0;
        test_critical_section::count = 0;
        log_one_formatted_rt_arg();
        CHECK(test_critical_section::count == 2);
        CHECK(log_calls == 1);
    }

    SECTION("log two runtime arguments") {
        log_calls = 0;
        test_critical_section::count = 0;
        log_two_rt_args();
        CHECK(test_critical_section::count == 2);
        CHECK(log_calls == 1);
    }

    // enum runtime arguments

    SECTION("log runtime scoped enum argument") {
        log_calls = 0;
        test_critical_section::count = 0;
        log_rt_scoped_enum_arg();
        CHECK(test_critical_section::count == 2);
        CHECK(log_calls == 1);
    }

    SECTION("log runtime unscoped enum argument") {
        log_calls = 0;
        test_critical_section::count = 0;
        log_rt_unscoped_enum_arg();
        CHECK(test_critical_section::count == 2);
        CHECK(log_calls == 1);
    }

    SECTION("log runtime scoped enum argument outside included header",
            "[catalog]") {
        log_calls = 0;
        test_critical_section::count = 0;
        log_rt_auto_scoped_enum_arg();
        CHECK(test_critical_section::count == 2);
        CHECK(log_calls == 1);
    }

    // float runtime arguments

    SECTION("log one float runtime argument") {
        log_calls = 0;
        test_critical_section::count = 0;
        log_rt_float_arg();
        CHECK(test_critical_section::count == 2);
        CHECK(log_calls == 1);
    }

    SECTION("log one double runtime argument") {
        log_calls = 0;
        test_critical_section::count = 0;
        log_rt_double_arg();
        CHECK(test_critical_section::count == 2);
        CHECK(log_calls == 1);
    }

    // environment functionality

    SECTION("log module ids change") {
        // subtype 1, severity 7, type 3
        std::uint32_t expected_static = (1u << 24u) | (7u << 4u) | 3u;
        log_with_default_module();
        CHECK((last_header & expected_static) == expected_static);

        auto default_header = last_header;
        log_with_non_default_module();
        CHECK((last_header & expected_static) == expected_static);
        CHECK((last_header ^ default_header) == (1u << 16u));
    }

    SECTION("log with stable module id") {
        std::uint32_t expected_static = (1u << 24u) | (7u << 4u) | 3u;

        log_with_fixed_module();
        CHECK((last_header & expected_static) == expected_static);
        // module ID 17 is fixed by stable_strings.json
        CHECK((last_header & ~expected_static) == (17u << 16u));
    }

    SECTION("log with fixed module id") {
        std::uint32_t expected_static = (1u << 24u) | (7u << 4u) | 3u;

        log_with_fixed_module_id();
        CHECK((last_header & expected_static) == expected_static);
        // module ID 6 is fixed by environment
        CHECK((last_header & ~expected_static) == (6u << 16u));
    }

    SECTION("log with fixed unsigned module id") {
        std::uint32_t expected_static = (1u << 24u) | (7u << 4u) | 3u;

        log_with_fixed_unsigned_module_id();
        CHECK((last_header & expected_static) == expected_static);
        // module ID 7 is fixed by environment
        CHECK((last_header & ~expected_static) == (7u << 16u));
    }
}
