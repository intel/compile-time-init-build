#include <match/ops.hpp>
#include <msg/field.hpp>
#include <msg/field_matchers.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using namespace msg;
using test_field =
    field<"test_field", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;
} // namespace

TEST_CASE("negate less_than", "[field matchers]") {
    constexpr auto m = msg::less_than_t<test_field, std::uint32_t, 5>{};
    constexpr auto n = match::negate(m);
    static_assert(
        std::is_same_v<decltype(n), msg::greater_than_or_equal_to_t<
                                        test_field, std::uint32_t, 5> const>);
}

TEST_CASE("negate greater_than", "[field matchers]") {
    constexpr auto m = msg::greater_than_t<test_field, std::uint32_t, 5>{};
    constexpr auto n = match::negate(m);
    static_assert(
        std::is_same_v<decltype(n), msg::less_than_or_equal_to_t<
                                        test_field, std::uint32_t, 5> const>);
}

TEST_CASE("negate less_than_or_equal_to", "[field matchers]") {
    constexpr auto m =
        msg::less_than_or_equal_to_t<test_field, std::uint32_t, 5>{};
    constexpr auto n = match::negate(m);
    static_assert(std::is_same_v<
                  decltype(n),
                  msg::greater_than_t<test_field, std::uint32_t, 5> const>);
}

TEST_CASE("negate greater_than_or_equal_to", "[field matchers]") {
    constexpr auto m =
        msg::greater_than_or_equal_to_t<test_field, std::uint32_t, 5>{};
    constexpr auto n = match::negate(m);
    static_assert(
        std::is_same_v<decltype(n),
                       msg::less_than_t<test_field, std::uint32_t, 5> const>);
}

TEST_CASE("less_than X implies less_than Y (X <= Y)", "[field matchers]") {
    constexpr auto m = msg::less_than_t<test_field, std::uint32_t, 5>{};
    constexpr auto n = msg::less_than_t<test_field, std::uint32_t, 6>{};
    static_assert(match::implies(m, n));
}

TEST_CASE("greater_than X implies greater_than Y (X >= Y)",
          "[field matchers]") {
    constexpr auto m = msg::greater_than_t<test_field, std::uint32_t, 6>{};
    constexpr auto n = msg::greater_than_t<test_field, std::uint32_t, 5>{};
    static_assert(match::implies(m, n));
}

TEST_CASE("less_than_or_equal_to X implies less_than Y (X < Y)",
          "[field matchers]") {
    constexpr auto m =
        msg::less_than_or_equal_to_t<test_field, std::uint32_t, 5>{};
    constexpr auto n = msg::less_than_t<test_field, std::uint32_t, 6>{};
    static_assert(match::implies(m, n));
}

TEST_CASE("less_than X implies less_than_or_equal_to Y (X <= Y + 1)",
          "[field matchers]") {
    constexpr auto m = msg::less_than_t<test_field, std::uint32_t, 6>{};
    constexpr auto n =
        msg::less_than_or_equal_to_t<test_field, std::uint32_t, 5>{};
    static_assert(match::implies(m, n));
}

TEST_CASE("greater_than_or_equal_to X implies greater_than Y (X > Y)",
          "[field matchers]") {
    constexpr auto m =
        msg::greater_than_or_equal_to_t<test_field, std::uint32_t, 6>{};
    constexpr auto n = msg::greater_than_t<test_field, std::uint32_t, 5>{};
    static_assert(match::implies(m, n));
}

TEST_CASE("greater_than X implies greater_than_or_equal_to Y (X + 1 >= Y)",
          "[field matchers]") {
    constexpr auto m = msg::greater_than_t<test_field, std::uint32_t, 5>{};
    constexpr auto n =
        msg::greater_than_or_equal_to_t<test_field, std::uint32_t, 6>{};
    static_assert(match::implies(m, n));
}

TEST_CASE("equal_to X implies less_than Y (X < Y)", "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, std::uint32_t, 5>{};
    constexpr auto n = msg::less_than_t<test_field, std::uint32_t, 6>{};
    static_assert(match::implies(m, n));
}

TEST_CASE("equal_to X implies less_than_or_equal_to Y (X <= Y)",
          "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, std::uint32_t, 5>{};
    constexpr auto n =
        msg::less_than_or_equal_to_t<test_field, std::uint32_t, 5>{};
    static_assert(match::implies(m, n));
}

TEST_CASE("equal_to X implies greater_than Y (X > Y)", "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, std::uint32_t, 6>{};
    constexpr auto n = msg::greater_than_t<test_field, std::uint32_t, 5>{};
    static_assert(match::implies(m, n));
}

TEST_CASE("equal_to X implies greater_than_or_equal_to Y (X >= Y)",
          "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, std::uint32_t, 5>{};
    constexpr auto n =
        msg::greater_than_or_equal_to_t<test_field, std::uint32_t, 5>{};
    static_assert(match::implies(m, n));
}

TEST_CASE("equal_to X implies not equal_to Y (X != Y)", "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, std::uint32_t, 5>{};
    constexpr auto n = not msg::equal_to_t<test_field, std::uint32_t, 6>{};
    static_assert(match::implies(m, n));
}

TEST_CASE("equal_to X and equal_to Y is false (X != Y)", "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, std::uint32_t, 5>{} and
                       msg::equal_to_t<test_field, std::uint32_t, 6>{};
    static_assert(std::is_same_v<decltype(m), match::never_t const>);
}

TEST_CASE("equal_to X and not equal_to Y is equal_to X (X != Y)",
          "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, std::uint32_t, 5>{} and
                       not msg::equal_to_t<test_field, std::uint32_t, 6>{};
    static_assert(
        std::is_same_v<decltype(m),
                       msg::equal_to_t<test_field, std::uint32_t, 5> const>);
}

TEST_CASE("equal_to X and less_than X is false (X != Y)", "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, std::uint32_t, 5>{} and
                       msg::less_than_t<test_field, std::uint32_t, 5>{};
    static_assert(std::is_same_v<decltype(m), match::never_t const>);
}

TEST_CASE("equal_to X and less_than Y is equal_to X (X < Y)",
          "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, std::uint32_t, 5>{} and
                       msg::less_than_t<test_field, std::uint32_t, 6>{};
    static_assert(
        std::is_same_v<decltype(m),
                       msg::equal_to_t<test_field, std::uint32_t, 5> const>);
}
