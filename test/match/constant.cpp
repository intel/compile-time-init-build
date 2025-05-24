#include <match/ops.hpp>

#include <stdx/ct_string.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("constant fulfils matcher concepts", "[match constant]") {
    using A = decltype(match::always);
    using N = decltype(match::never);
    STATIC_REQUIRE(match::matcher<A>);
    STATIC_REQUIRE(match::matcher<N>);
    STATIC_REQUIRE(match::matcher_for<A, int>);
    STATIC_REQUIRE(match::matcher_for<N, int>);
}

TEST_CASE("constant describes itself", "[match constant]") {
    using namespace stdx::literals;
    using A = decltype(match::always);
    using N = decltype(match::never);
    STATIC_REQUIRE(A{}.describe() == "true"_ctst);
    STATIC_REQUIRE(N{}.describe() == "false"_ctst);
}

TEST_CASE("constant describes a match", "[match not]") {
    using namespace stdx::literals;
    using A = decltype(match::always);
    using N = decltype(match::never);
    STATIC_REQUIRE(A{}.describe_match(0) == "true"_ctst);
    STATIC_REQUIRE(N{}.describe_match(0) == "false"_ctst);
}

TEST_CASE("constant matches correctly", "[match constant]") {
    STATIC_REQUIRE(match::always(0));
    STATIC_REQUIRE(not match::never(0));
}
