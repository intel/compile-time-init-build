#include <cib/cib.hpp>

#include <catch2/catch_test_macros.hpp>

using cib::detail::int_;
using cib::detail::ordered_set;

template<int Value>
using int_t = std::integral_constant<int, Value>;

TEST_CASE("make empty set", "[ordered_set]") {
    auto const t = ordered_set();
    REQUIRE(t.size() == 0);
}

TEST_CASE("make set with a single element", "[ordered_set]") {
    auto const t = ordered_set(int{54});
    REQUIRE(t.size() == 1);
    REQUIRE(t.get<0>() == 54);
    REQUIRE(t.get<int>() == 54);
}

TEST_CASE("make set with a multiple elements", "[ordered_set]") {
    auto const t = ordered_set(int{54}, long{12}, bool{false}, char{'3'});

    REQUIRE(t.size() == 4);

    REQUIRE(t.get<0>() == 54);
    REQUIRE(t.get<1>() == 12);
    REQUIRE(t.get<2>() == false);
    REQUIRE(t.get<3>() == '3');

    REQUIRE(t.get<int>() == 54);
    REQUIRE(t.get<long>() == 12);
    REQUIRE(t.get<bool>() == false);
    REQUIRE(t.get<char>() == '3');
}

TEST_CASE("compare equality of empty sets", "[ordered_set]") {
    auto const lhs = ordered_set();
    auto const rhs = ordered_set();

    REQUIRE(lhs == rhs);
    REQUIRE_FALSE(lhs != rhs);
}

TEST_CASE("compare equality of sets with different types", "[ordered_set]") {
    auto const lhs = ordered_set();
    auto const rhs = ordered_set(12);

    REQUIRE_FALSE(lhs == rhs);
    REQUIRE(lhs != rhs);
}

TEST_CASE("compare equality of sets with same types and values", "[ordered_set]") {
    auto const lhs = ordered_set(12, true, 'c');
    auto const rhs = ordered_set(12, true, 'c');

    REQUIRE(lhs == rhs);
    REQUIRE_FALSE(lhs != rhs);
}

TEST_CASE("compare equality of sets with same types and different values", "[ordered_set]") {
    auto const lhs = ordered_set(3, true, 'c');
    auto const rhs = ordered_set(12, true, 'c');

    REQUIRE_FALSE(lhs == rhs);
    REQUIRE(lhs != rhs);
}

TEST_CASE("concatenate two sets", "[ordered_set]") {
    auto const t = 
        cib::detail::tuple_cat(
            ordered_set(3),
            ordered_set('c'));

    REQUIRE(t == ordered_set(3, 'c'));
}

TEST_CASE("concatenate empty set with non-empty set", "[ordered_set]") {
    auto const t =
        cib::detail::tuple_cat(
            ordered_set(3),
            ordered_set());

    REQUIRE(t == ordered_set(3));
}


