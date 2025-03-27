#include <match/concepts.hpp>
#include <match/predicate.hpp>

#include <stdx/ct_string.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("predicate fulfils matcher concepts", "[match predicate]") {
    [[maybe_unused]] constexpr auto p =
        match::predicate<"P">([](int) { return true; });
    static_assert(match::matcher<decltype(p)>);
    static_assert(match::matcher_for<decltype(p), int>);
    static_assert(not match::matcher_for<decltype(p), decltype([] {})>);
}

TEST_CASE("predicate describes itself", "[match predicate]") {
    using namespace stdx::literals;
    [[maybe_unused]] constexpr auto p =
        match::predicate<"P">([](int) { return true; });
    static_assert(p.describe() == "P"_ctst);
}

TEST_CASE("unnamed predicate", "[match predicate]") {
    using namespace stdx::literals;
    [[maybe_unused]] constexpr auto p =
        match::predicate([](int) { return true; });
    static_assert(p.describe() == "<predicate>"_ctst);
}

TEST_CASE("predicate matches correctly", "[match predicate]") {
    constexpr auto p = match::predicate([](int i) { return i % 2 == 0; });
    static_assert(p(0));
    static_assert(not p(1));
}
