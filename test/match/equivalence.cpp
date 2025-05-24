#include "test_matcher.hpp"

#include <match/ops.hpp>

#include <catch2/catch_test_macros.hpp>

#include <compare>
#include <type_traits>

TEST_CASE("less than", "[match equivalence]") {
    using T = match::and_t<test_m<0>, test_m<1>>;
    using U = test_m<0>;
    constexpr auto result = T{} <=> U{};
    STATIC_REQUIRE(result == std::partial_ordering::less);
    STATIC_REQUIRE(T{} < U{});
}

TEST_CASE("greater than", "[match equivalence]") {
    using T = match::or_t<test_m<0>, test_m<1>>;
    using U = test_m<0>;
    constexpr auto result = T{} <=> U{};
    STATIC_REQUIRE(result == std::partial_ordering::greater);
    STATIC_REQUIRE(T{} > U{});
}

TEST_CASE("equivalent", "[match equivalence]") {
    using T = test_m<0>;
    constexpr auto result = T{} <=> T{};
    STATIC_REQUIRE(result == std::partial_ordering::equivalent);
}

TEST_CASE("unorderable", "[match equivalence]") {
    using T = test_m<0>;
    using U = test_m<1>;
    constexpr auto result = T{} <=> U{};
    STATIC_REQUIRE(result == std::partial_ordering::unordered);
}
