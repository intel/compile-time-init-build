#include <match/ops.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("constant fulfils matcher concepts", "[match constant]") {
    using A = decltype(match::always);
    using N = decltype(match::never);
    static_assert(match::matcher<A>);
    static_assert(match::matcher<N>);
    static_assert(match::matcher_for<A, int>);
    static_assert(match::matcher_for<N, int>);
}

TEST_CASE("constant describes itself", "[match constant]") {
    using A = decltype(match::always);
    using N = decltype(match::never);
    static_assert(A{}.describe() == "true"_sc);
    static_assert(N{}.describe() == "false"_sc);
}

TEST_CASE("constant describes a match", "[match not]") {
    using A = decltype(match::always);
    using N = decltype(match::never);
    static_assert(A{}.describe_match(0) == "true"_sc);
    static_assert(N{}.describe_match(0) == "false"_sc);
}

TEST_CASE("constant matches correctly", "[match constant]") {
    static_assert(match::always(0));
    static_assert(not match::never(0));
}
