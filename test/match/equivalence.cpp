#include "test_matcher.hpp"

#include <match/ops.hpp>

#include <catch2/catch_test_macros.hpp>

#include <compare>
#include <type_traits>

TEST_CASE("less than", "[match equivalence]") {
    using T = test_m<0>;
    using U = match::and_t<T, test_m<1>>;
    constexpr auto result = T{} <=> U{};
    static_assert(result == std::partial_ordering::less);
    static_assert(T{} < U{});
}

TEST_CASE("greater than", "[match equivalence]") {
    using T = test_m<0>;
    using U = match::or_t<T, test_m<1>>;
    constexpr auto result = T{} <=> U{};
    static_assert(result == std::partial_ordering::greater);
    static_assert(T{} > U{});
}

TEST_CASE("equivalent", "[match equivalence]") {
    using T = test_m<0>;
    constexpr auto result = T{} <=> T{};
    static_assert(result == std::partial_ordering::equivalent);
}

TEST_CASE("unorderable", "[match equivalence]") {
    using T = test_m<0>;
    using U = test_m<1>;
    constexpr auto result = T{} <=> U{};
    static_assert(result == std::partial_ordering::unordered);
}
