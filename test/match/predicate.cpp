#include <match/concepts.hpp>
#include <match/predicate.hpp>

#include <stdx/ct_string.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("predicate fulfils matcher concepts", "[match predicate]") {
    [[maybe_unused]] constexpr auto p =
        match::predicate<"P">([](int) { return true; });
    STATIC_REQUIRE(match::matcher<decltype(p)>);
    STATIC_REQUIRE(match::matcher_for<decltype(p), int>);
    STATIC_REQUIRE(not match::matcher_for<decltype(p), decltype([] {})>);
}

TEST_CASE("predicate describes itself", "[match predicate]") {
    using namespace stdx::literals;
    [[maybe_unused]] constexpr auto p =
        match::predicate<"P">([](int) { return true; });
    STATIC_REQUIRE(p.describe() == "P"_ctst);
}

TEST_CASE("unnamed predicate", "[match predicate]") {
    using namespace stdx::literals;
    [[maybe_unused]] constexpr auto p =
        match::predicate([](int) { return true; });
    STATIC_REQUIRE(p.describe() == "<predicate>"_ctst);
}

TEST_CASE("predicate matches correctly", "[match predicate]") {
    constexpr auto p = match::predicate([](int i) { return i % 2 == 0; });
    STATIC_REQUIRE(p(0));
    STATIC_REQUIRE(not p(1));
}
