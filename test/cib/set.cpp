#include <cib/cib.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>


using cib::tag_;
using cib::index_;
using cib::index_metafunc_;

template<int Value>
using int_t = std::integral_constant<int, Value>;

struct A {};
struct B {};
struct C {};
struct D {};
struct E {};
struct F {};
struct G {};
struct H {};
struct I {};
struct J {};
struct K {};
struct L {};




TEST_CASE("union empty sets", "[set]") {
    auto t = cib::set_union(
        cib::make_tuple(),
        cib::make_tuple()
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple())>));
}

TEST_CASE("union tiny sets", "[set]") {
    auto t = cib::set_union(
        cib::make_tuple(A{}),
        cib::make_tuple(B{})
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple(A{}, B{}))>));
}

TEST_CASE("union tiny sets with same types", "[set]") {
    auto t = cib::set_union(
        cib::make_tuple(A{}),
        cib::make_tuple(A{})
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple(A{}))>));
}

TEST_CASE("union less tiny sets", "[set]") {
    auto t = cib::set_union(
        cib::make_tuple(A{}, C{}, E{}),
        cib::make_tuple(B{}, D{}, F{})
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple(A{}, B{}, C{}, D{}, E{}, F{}))>));
}

TEST_CASE("union less tiny sets with same types", "[set]") {
    auto t = cib::set_union(
        cib::make_tuple(A{}, B{}, C{}),
        cib::make_tuple(A{}, B{}, C{})
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple(A{}, B{}, C{}))>));
}




TEST_CASE("intersection empty sets", "[set]") {
    auto t = cib::set_intersection(
        cib::make_tuple(),
        cib::make_tuple()
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple())>));
}

TEST_CASE("intersection tiny sets, same", "[set]") {
    auto t = cib::set_intersection(
        cib::make_tuple(A{}),
        cib::make_tuple(A{})
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple(A{}))>));
}

TEST_CASE("intersection tiny sets, different", "[set]") {
    auto t = cib::set_intersection(
        cib::make_tuple(A{}),
        cib::make_tuple(B{})
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple())>));
}

TEST_CASE("intersection less tiny sets", "[set]") {
    auto t = cib::set_intersection(
        cib::make_tuple(A{}, C{}, E{}),
        cib::make_tuple(B{}, D{}, F{})
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple())>));
}

TEST_CASE("intersection less tiny sets with same types", "[set]") {
    auto t = cib::set_intersection(
        cib::make_tuple(A{}, B{}, C{}),
        cib::make_tuple(A{}, B{}, C{})
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple(A{}, B{}, C{}))>));
}


TEST_CASE("difference empty sets", "[set]") {
    auto t = cib::set_difference(
        cib::make_tuple(),
        cib::make_tuple()
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple())>));
}

TEST_CASE("difference one empty set", "[set]") {
    auto t = cib::set_difference(
        cib::make_tuple(A{}),
        cib::make_tuple()
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple(A{}))>));
}

TEST_CASE("difference other empty set", "[set]") {
    auto t = cib::set_difference(
        cib::make_tuple(),
        cib::make_tuple(A{})
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple())>));
}

TEST_CASE("difference first item", "[set]") {
    auto t = cib::set_difference(
        cib::make_tuple(A{}, B{}),
        cib::make_tuple(A{})
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple(B{}))>));
}

TEST_CASE("difference last item", "[set]") {
    auto t = cib::set_difference(
        cib::make_tuple(A{}, B{}),
        cib::make_tuple(B{})
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple(A{}))>));
}


TEST_CASE("symmetric difference empty sets", "[set]") {
    auto t = cib::set_symmetric_difference(
        cib::make_tuple(),
        cib::make_tuple()
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple())>));
}

TEST_CASE("symmetric difference left empty set", "[set]") {
    auto t = cib::set_symmetric_difference(
        cib::make_tuple(),
        cib::make_tuple(A{})
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple(A{}))>));
}

TEST_CASE("symmetric difference right empty set", "[set]") {
    auto t = cib::set_symmetric_difference(
        cib::make_tuple(A{}),
        cib::make_tuple()
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple(A{}))>));
}

TEST_CASE("symmetric difference both same", "[set]") {
    auto t = cib::set_symmetric_difference(
        cib::make_tuple(A{}),
        cib::make_tuple(A{})
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple())>));
}

TEST_CASE("symmetric difference both different", "[set]") {
    auto t = cib::set_symmetric_difference(
        cib::make_tuple(A{}),
        cib::make_tuple(B{})
    );

    REQUIRE((std::is_same_v<decltype(t), decltype(cib::make_tuple(A{}, B{}))>));
}

