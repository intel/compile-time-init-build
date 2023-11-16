#include <interrupt/policies.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("policies get", "[policies]") {
    using policies_t = interrupt::policies<interrupt::clear_status_first>;
    using default_policy_t = interrupt::clear_status_last;
    static_assert(
        std::is_same_v<decltype(policies_t::get<interrupt::status_clear_policy,
                                                default_policy_t>()),
                       interrupt::clear_status_first>);
    static_assert(std::is_same_v<
                  decltype(policies_t::get<interrupt::required_resources_policy,
                                           default_policy_t>()),
                  default_policy_t>);
}

TEST_CASE("clear status first policy", "[policies]") {
    int call_value{};
    interrupt::clear_status_first::run([&] { ++call_value; },
                                       [&] { call_value *= 2; });
    CHECK(call_value == 2);
}

TEST_CASE("clear status last policy", "[policies]") {
    int call_value{};
    interrupt::clear_status_last::run([&] { ++call_value; },
                                      [&] { call_value *= 2; });
    CHECK(call_value == 1);
}

TEST_CASE("don't clear status policy", "[policies]") {
    int call_value{};
    interrupt::dont_clear_status::run([&] { call_value += 3; },
                                      [&] { ++call_value; });
    CHECK(call_value == 1);
}
