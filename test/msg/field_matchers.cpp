#include <match/ops.hpp>
#include <msg/field.hpp>
#include <msg/field_matchers.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using namespace msg;
using test_field =
    field<"test_field", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;

enum struct E { A, B, C };

using test_enum_field =
    field<"enum_field", E>::located<at{0_dw, 31_msb, 24_lsb}>;
} // namespace

TEST_CASE("matcher description", "[field matchers]") {
    using namespace stdx::literals;
    constexpr auto m = msg::less_than_t<test_field, 5>{};
    constexpr auto desc = m.describe();
    STATIC_REQUIRE(desc.str == "test_field < 0x5"_ctst);
}

TEST_CASE("matcher description of match", "[field matchers]") {
    using namespace stdx::literals;
    using msg_data = std::array<std::uint32_t, 1>;

    constexpr auto m = msg::less_than_t<test_field, 5>{};
    constexpr auto desc = m.describe_match(msg_data{0x01ff'ffff});
    STATIC_REQUIRE(desc.str == "test_field (0x{:x}) < 0x5"_ctst);
    STATIC_REQUIRE(desc.args == stdx::tuple{1});
}

TEST_CASE("matcher description (enum field)", "[field matchers]") {
    using namespace stdx::literals;
    constexpr auto m = msg::less_than_t<test_enum_field, E::C>{};
    constexpr auto desc = m.describe();
    STATIC_REQUIRE(desc.str == "enum_field < C"_ctst);
}

TEST_CASE("matcher description of match (enum field)", "[field matchers]") {
    using namespace stdx::literals;
    using msg_data = std::array<std::uint32_t, 1>;

    constexpr auto m = msg::less_than_t<test_enum_field, E::C>{};
    constexpr auto desc = m.describe_match(msg_data{0x01ff'ffff});
    STATIC_REQUIRE(desc.str == "enum_field ({}) < C"_ctst);
    STATIC_REQUIRE(desc.args == stdx::tuple{E::B});
}

TEST_CASE("negate less_than", "[field matchers]") {
    constexpr auto m = msg::less_than_t<test_field, 5>{};
    constexpr auto n = match::negate(m);
    STATIC_REQUIRE(
        std::is_same_v<decltype(n),
                       msg::greater_than_or_equal_to_t<test_field, 5> const>);
}

TEST_CASE("negate greater_than", "[field matchers]") {
    constexpr auto m = msg::greater_than_t<test_field, 5>{};
    constexpr auto n = match::negate(m);
    STATIC_REQUIRE(
        std::is_same_v<decltype(n),
                       msg::less_than_or_equal_to_t<test_field, 5> const>);
}

TEST_CASE("negate less_than_or_equal_to", "[field matchers]") {
    constexpr auto m = msg::less_than_or_equal_to_t<test_field, 5>{};
    constexpr auto n = match::negate(m);
    STATIC_REQUIRE(
        std::is_same_v<decltype(n), msg::greater_than_t<test_field, 5> const>);
}

TEST_CASE("negate greater_than_or_equal_to", "[field matchers]") {
    constexpr auto m = msg::greater_than_or_equal_to_t<test_field, 5>{};
    constexpr auto n = match::negate(m);
    STATIC_REQUIRE(
        std::is_same_v<decltype(n), msg::less_than_t<test_field, 5> const>);
}

TEST_CASE("negate equal_to", "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, 5>{};
    constexpr auto n = match::negate(m);
    STATIC_REQUIRE(
        std::is_same_v<decltype(n), msg::not_equal_to_t<test_field, 5> const>);
}

TEST_CASE("negate not_equal_to", "[field matchers]") {
    constexpr auto m = msg::not_equal_to_t<test_field, 5>{};
    constexpr auto n = match::negate(m);
    STATIC_REQUIRE(
        std::is_same_v<decltype(n), msg::equal_to_t<test_field, 5> const>);
}

TEST_CASE("less_than X implies less_than Y (X == Y)", "[field matchers]") {
    constexpr auto m = msg::less_than_t<test_field, 5>{};
    constexpr auto n = msg::less_than_t<test_field, 5>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("less_than X implies less_than Y (X <= Y)", "[field matchers]") {
    constexpr auto m = msg::less_than_t<test_field, 5>{};
    constexpr auto n = msg::less_than_t<test_field, 6>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("greater_than X implies greater_than Y (X >= Y)",
          "[field matchers]") {
    constexpr auto m = msg::greater_than_t<test_field, 6>{};
    constexpr auto n = msg::greater_than_t<test_field, 5>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("less_than_or_equal_to X implies less_than Y (X < Y)",
          "[field matchers]") {
    constexpr auto m = msg::less_than_or_equal_to_t<test_field, 5>{};
    constexpr auto n = msg::less_than_t<test_field, 6>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("less_than X implies less_than_or_equal_to Y (X <= Y + 1)",
          "[field matchers]") {
    constexpr auto m = msg::less_than_t<test_field, 6>{};
    constexpr auto n = msg::less_than_or_equal_to_t<test_field, 5>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("greater_than_or_equal_to X implies greater_than Y (X > Y)",
          "[field matchers]") {
    constexpr auto m = msg::greater_than_or_equal_to_t<test_field, 6>{};
    constexpr auto n = msg::greater_than_t<test_field, 5>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("greater_than X implies greater_than_or_equal_to Y (X + 1 >= Y)",
          "[field matchers]") {
    constexpr auto m = msg::greater_than_t<test_field, 5>{};
    constexpr auto n = msg::greater_than_or_equal_to_t<test_field, 6>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("equal_to X implies less_than Y (X < Y)", "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, 5>{};
    constexpr auto n = msg::less_than_t<test_field, 6>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("equal_to X implies less_than_or_equal_to Y (X <= Y)",
          "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, 5>{};
    constexpr auto n = msg::less_than_or_equal_to_t<test_field, 5>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("equal_to X implies greater_than Y (X > Y)", "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, 6>{};
    constexpr auto n = msg::greater_than_t<test_field, 5>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("equal_to X implies greater_than_or_equal_to Y (X >= Y)",
          "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, 5>{};
    constexpr auto n = msg::greater_than_or_equal_to_t<test_field, 5>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("equal_to X implies not equal_to Y (X != Y)", "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, 5>{};
    constexpr auto n = not msg::equal_to_t<test_field, 6>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("equal_to X and equal_to Y is false (X != Y)", "[field matchers]") {
    constexpr auto m =
        msg::equal_to_t<test_field, 5>{} and msg::equal_to_t<test_field, 6>{};
    STATIC_REQUIRE(std::is_same_v<decltype(m), match::never_t const>);
}

TEST_CASE("equal_to X and not equal_to Y is equal_to X (X != Y)",
          "[field matchers]") {
    constexpr auto m = msg::equal_to_t<test_field, 5>{} and
                       not msg::equal_to_t<test_field, 6>{};
    STATIC_REQUIRE(
        std::is_same_v<decltype(m), msg::equal_to_t<test_field, 5> const>);
}

TEST_CASE("equal_to X and less_than X is false (X != Y)", "[field matchers]") {
    constexpr auto m =
        msg::equal_to_t<test_field, 5>{} and msg::less_than_t<test_field, 5>{};
    STATIC_REQUIRE(std::is_same_v<decltype(m), match::never_t const>);
}

TEST_CASE("equal_to X and less_than Y is equal_to X (X < Y)",
          "[field matchers]") {
    constexpr auto m =
        msg::equal_to_t<test_field, 5>{} and msg::less_than_t<test_field, 6>{};
    STATIC_REQUIRE(
        std::is_same_v<decltype(m), msg::equal_to_t<test_field, 5> const>);
}

TEST_CASE("less_than X implies not_equal_to X", "[field matchers]") {
    constexpr auto m = msg::less_than_t<test_field, 5>{};
    constexpr auto n = msg::not_equal_to_t<test_field, 5>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("greater_than X implies not_equal_to X", "[field matchers]") {
    constexpr auto m = msg::greater_than_t<test_field, 5>{};
    constexpr auto n = msg::not_equal_to_t<test_field, 5>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("less_than_or_equal_to X implies not_equal_to X",
          "[field matchers]") {
    constexpr auto m = msg::less_than_or_equal_to_t<test_field, 5>{};
    constexpr auto n = msg::not_equal_to_t<test_field, 6>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("greater_than_or_equal_to X implies not_equal_to X",
          "[field matchers]") {
    constexpr auto m = msg::greater_than_or_equal_to_t<test_field, 6>{};
    constexpr auto n = msg::not_equal_to_t<test_field, 5>{};
    STATIC_REQUIRE(match::implies(m, n));
}

TEST_CASE("not_equal_to X and less_than Y is less_than Y (X >= Y)",
          "[field matchers]") {
    constexpr auto m = msg::not_equal_to_t<test_field, 6>{} and
                       msg::less_than_t<test_field, 6>{};
    STATIC_REQUIRE(
        std::is_same_v<decltype(m), msg::less_than_t<test_field, 6> const>);
}

TEST_CASE("predicate matcher description", "[field matchers]") {
    using namespace stdx::literals;
    constexpr auto m =
        msg::pred_matcher_t<test_field, [](std::uint32_t) { return true; }>{};
    constexpr auto desc = m.describe();
    STATIC_REQUIRE(desc.str == "<predicate>(test_field)"_ctst);
}

TEST_CASE("predicate matcher description of match", "[field matchers]") {
    using namespace stdx::literals;
    using msg_data = std::array<std::uint32_t, 1>;

    constexpr auto m =
        msg::pred_matcher_t<test_field, [](std::uint32_t) { return true; }>{};
    constexpr auto desc = m.describe_match(msg_data{0x01ff'ffff});
    STATIC_REQUIRE(desc.str == "<predicate>(test_field(0x{:x}))"_ctst);
    STATIC_REQUIRE(desc.args == stdx::tuple{1});
}

TEST_CASE("predicate matcher description of match (enum field)",
          "[field matchers]") {
    using namespace stdx::literals;
    using msg_data = std::array<std::uint32_t, 1>;

    constexpr auto m =
        msg::pred_matcher_t<test_enum_field, [](auto) { return true; }>{};
    constexpr auto desc = m.describe_match(msg_data{0x01ff'ffff});
    STATIC_REQUIRE(desc.str == "<predicate>(enum_field({}))"_ctst);
    STATIC_REQUIRE(desc.args == stdx::tuple{E::B});
}
