#include <msg/match.hpp>
#include <sc/string_constant.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
template <bool Value> struct always_t {
    template <typename T>
    [[nodiscard]] constexpr bool operator()(T const &event) const {
        return Value;
    }

    [[nodiscard]] constexpr auto describe() const {
        if constexpr (Value) {
            return "true"_sc;
        } else {
            return "false"_sc;
        }
    }

    template <typename T>
    [[nodiscard]] constexpr auto describe_match(T const &event) const {
        if constexpr (Value) {
            return "true"_sc;
        } else {
            return "false"_sc;
        }
    }
};

template <bool Value> static constexpr always_t<Value> always{};

template <int Value> struct Number {
    [[nodiscard]] constexpr bool operator()(int const &event) const {
        return event == Value;
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("number == {}"_sc, sc::int_<Value>);
    }

    [[nodiscard]] constexpr auto describe_match(int const &event) const {
        return format("number ({}) == {}"_sc, event, sc::int_<Value>);
    }
};

template <int Value> static constexpr Number<Value> number{};

TEST_CASE("MatchAny", "[match]") {
    REQUIRE_FALSE(match::any()(0));

    REQUIRE_FALSE(match::any(always<false>)(0));
    REQUIRE(match::any(always<true>)(0));

    REQUIRE_FALSE(match::any(always<false>, always<false>)(0));
    REQUIRE(match::any(always<false>, always<true>)(0));
    REQUIRE(match::any(always<true>, always<false>)(0));
    REQUIRE(match::any(always<true>, always<true>)(0));

    REQUIRE_FALSE(match::any(always<false>, always<false>, always<false>)(0));
    REQUIRE(match::any(always<true>, always<false>, always<false>)(0));
    REQUIRE(match::any(always<false>, always<true>, always<false>)(0));
    REQUIRE(match::any(always<false>, always<false>, always<true>)(0));
}

TEST_CASE("MatchAnyWithEvents", "[match]") {
    REQUIRE(match::any(number<1>)(1));
    REQUIRE_FALSE(match::any(number<1>)(5));
    REQUIRE_FALSE(match::any(number<1>, number<4>)(5));
    REQUIRE(match::any(number<1>, number<4>)(4));
    REQUIRE_FALSE(match::any(number<1>, number<2>, number<3>)(4));
    REQUIRE(match::any(number<3>, number<2>, number<8>)(3));
}

TEST_CASE("MatchAnyDescription", "[match]") {
    REQUIRE(match::any(number<1>).describe() == "number == 1"_sc);

    REQUIRE(match::any(number<1>, number<4>).describe() ==
            "(number == 1) || (number == 4)"_sc);

    REQUIRE(match::any(number<1>, number<2>, number<3>).describe() ==
            "(number == 1) || (number == 2) || (number == 3)"_sc);
}

TEST_CASE("MatchAnyDescriptionWithEvent", "[match]") {
    REQUIRE(match::any(number<1>).describe_match(1) ==
            format("number ({}) == 1"_sc, 1));

    REQUIRE(match::any(number<1>, number<4>).describe_match(0) ==
            format("{:c}:(number ({}) == 1) || {:c}:(number ({}) == 4)"_sc, 'F',
                   0, 'F', 0));

    REQUIRE(
        match::any(number<1>, number<2>, number<3>).describe_match(2) ==
        format(
            "{:c}:(number ({}) == 1) || {:c}:(number ({}) == 2) || {:c}:(number ({}) == 3)"_sc,
            'F', 2, 'T', 2, 'F', 2));
}

TEST_CASE("MatchAll", "[match]") {
    REQUIRE(match::all()(0));

    REQUIRE_FALSE(match::all(always<false>)(0));
    REQUIRE(match::all(always<true>)(0));

    REQUIRE_FALSE(match::all(always<false>, always<false>)(0));
    REQUIRE_FALSE(match::all(always<false>, always<true>)(0));
    REQUIRE_FALSE(match::all(always<true>, always<false>)(0));
    REQUIRE(match::all(always<true>, always<true>)(0));

    REQUIRE_FALSE(match::all(always<false>, always<false>, always<false>)(0));
    REQUIRE_FALSE(match::all(always<true>, always<false>, always<false>)(0));
    REQUIRE_FALSE(match::all(always<false>, always<true>, always<false>)(0));
    REQUIRE_FALSE(match::all(always<false>, always<false>, always<true>)(0));
    REQUIRE(match::all(always<true>, always<true>, always<true>)(0));
}

TEST_CASE("MatchAllWithEvents", "[match]") {
    REQUIRE_FALSE(match::all(number<0>, number<1>, number<2>)(0));
    REQUIRE_FALSE(match::all(number<0>, number<1>, number<2>)(1));
    REQUIRE_FALSE(match::all(number<0>, number<1>, number<2>)(2));
    REQUIRE_FALSE(match::all(number<0>, number<1>, number<2>)(3));
    REQUIRE_FALSE(match::all(number<0>, number<0>, number<2>)(0));
    REQUIRE_FALSE(match::all(number<0>, number<2>, number<0>)(0));
    REQUIRE_FALSE(match::all(number<2>, number<0>, number<0>)(0));
    REQUIRE(match::all(number<0>, number<0>, number<0>)(0));
    REQUIRE(match::all(number<8>, number<8>)(8));
    REQUIRE(match::all(number<2>)(2));
}

TEST_CASE("MatchAllDescription", "[match]") {
    REQUIRE(match::all(number<1>).describe() == "number == 1"_sc);

    REQUIRE(match::all(number<1>, number<4>).describe() ==
            "(number == 1) && (number == 4)"_sc);

    REQUIRE(match::all(number<1>, number<2>, number<3>).describe() ==
            "(number == 1) && (number == 2) && (number == 3)"_sc);
}

TEST_CASE("MatchAllDescriptionWithEvent", "[match]") {
    REQUIRE(match::all(number<1>).describe_match(1) ==
            format("number ({}) == 1"_sc, 1));

    REQUIRE(match::all(number<1>, number<4>).describe_match(0) ==
            format("{:c}:(number ({}) == 1) && {:c}:(number ({}) == 4)"_sc, 'F',
                   0, 'F', 0));

    REQUIRE(
        match::all(number<1>, number<2>, number<3>).describe_match(2) ==
        format(
            "{:c}:(number ({}) == 1) && {:c}:(number ({}) == 2) && {:c}:(number ({}) == 3)"_sc,
            'F', 2, 'T', 2, 'F', 2));
}
} // namespace
