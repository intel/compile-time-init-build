#include <cib/cib.hpp>

#include <catch2/catch_test_macros.hpp>

using cib::tag_;
using cib::index_;
using cib::index_metafunc_;

template<int Value>
using int_t = std::integral_constant<int, Value>;

struct A {};
struct B {};
struct C {};
struct D {};

template<typename KeyT, typename ValueT>
struct map_entry {
    using Key = KeyT;
    using Value = ValueT;

    ValueT value;
};

TEST_CASE("make empty cib::tuple", "[tuple]") {
    auto const t = cib::make_tuple();
    REQUIRE(t.size() == 0);
}

TEST_CASE("single make_tuple", "[tuple]") {
    auto const t = cib::make_tuple(42);

    REQUIRE(t.size() == 1);
    REQUIRE(t.get(index_<0>) == 42);
}

TEST_CASE("simple make_tuple", "[tuple]") {
    auto const t = cib::make_tuple(5, 10);

    REQUIRE(t.size() == 2);
    REQUIRE(t.get(index_<0>) == 5);
    REQUIRE(t.get(index_<1>) == 10);
}

TEST_CASE("self_type_index", "[tuple]") {
    auto const t = cib::make_tuple(
        cib::self_type_index,
        5,
        false,
        10l
    );

    REQUIRE(t.size() == 3);
    REQUIRE(t.get(index_<0>) == 5);
    REQUIRE(t.get(index_<1>) == false);
    REQUIRE(t.get(index_<2>) == 10);

    REQUIRE(t.get(tag_<int>) == 5);
    REQUIRE(t.get(tag_<bool>) == false);
    REQUIRE(t.get(tag_<long>) == 10);
}


struct extract_key {
    template<typename T>
    using invoke = typename T::Key;
};

TEST_CASE("make empty tuple with calculated index", "[tuple]") {
    auto const t =
        cib::make_tuple(
            index_metafunc_<extract_key>
        );

    REQUIRE(t.size() == 0);
}

TEST_CASE("make_tuple with calculated index", "[tuple]") {
    auto const t =
        cib::make_tuple(
            index_metafunc_<extract_key>,
            map_entry<A, int>{42},
            map_entry<B, int>{12},
            map_entry<C, int>{55},
            map_entry<D, int>{99}
        );

    REQUIRE(t.size() == 4);

    REQUIRE(t.get(index_<0>).value == 42);
    REQUIRE(t.get(index_<1>).value == 12);
    REQUIRE(t.get(index_<2>).value == 55);
    REQUIRE(t.get(index_<3>).value == 99);

    REQUIRE(t.get(tag_<A>).value == 42);
    REQUIRE(t.get(tag_<B>).value == 12);
    REQUIRE(t.get(tag_<C>).value == 55);
    REQUIRE(t.get(tag_<D>).value == 99);
}


TEST_CASE("tuple_cat", "[tuple]") {
    auto const t0 = cib::make_tuple(5, 10);
    auto const t1 = cib::make_tuple(12, 30);

    auto const t = tuple_cat(t0, t1);

    REQUIRE(t.size() == 4);
    REQUIRE(t.get(index_<0>) == 5);
    REQUIRE(t.get(index_<1>) == 10);
    REQUIRE(t.get(index_<2>) == 12);
    REQUIRE(t.get(index_<3>) == 30);
}

TEST_CASE("tuple_cat left empty", "[tuple]") {
    auto const t0 = cib::make_tuple();
    auto const t1 = cib::make_tuple(12, 30);

    auto const t = tuple_cat(t0, t1);

    REQUIRE(t.size() == 2);
    REQUIRE(t.get(index_<0>) == 12);
    REQUIRE(t.get(index_<1>) == 30);
}

TEST_CASE("tuple_cat right empty", "[tuple]") {
    auto const t0 = cib::make_tuple(12, 30);
    auto const t1 = cib::make_tuple();

    auto const t = tuple_cat(t0, t1);

    REQUIRE(t.size() == 2);
    REQUIRE(t.get(index_<0>) == 12);
    REQUIRE(t.get(index_<1>) == 30);
}

TEST_CASE("tuple_cat both empty", "[tuple]") {
    auto const t0 = cib::make_tuple();
    auto const t1 = cib::make_tuple();

    auto const t = tuple_cat(t0, t1);

    REQUIRE(t.size() == 0);
}

TEST_CASE("tuple_cat three empty", "[tuple]") {
    auto const t0 = cib::make_tuple();
    auto const t1 = cib::make_tuple();
    auto const t2 = cib::make_tuple();

    auto const t = tuple_cat(t0, t1, t2);

    REQUIRE(t.size() == 0);
}

TEST_CASE("tuple_cat middle empty", "[tuple]") {
    auto const t0 = cib::make_tuple(1, 2);
    auto const t1 = cib::make_tuple();
    auto const t2 = cib::make_tuple(3, 4);

    auto const t = tuple_cat(t0, t1, t2);

    REQUIRE(t.size() == 4);
    REQUIRE(t.get(index_<0>) == 1);
    REQUIRE(t.get(index_<1>) == 2);
    REQUIRE(t.get(index_<2>) == 3);
    REQUIRE(t.get(index_<3>) == 4);
}

TEST_CASE("tuple_cat first empty", "[tuple]") {
    auto const t0 = cib::make_tuple();
    auto const t1 = cib::make_tuple(1, 2);
    auto const t2 = cib::make_tuple(3, 4);

    auto const t = tuple_cat(t0, t1, t2);

    REQUIRE(t.size() == 4);
    REQUIRE(t.get(index_<0>) == 1);
    REQUIRE(t.get(index_<1>) == 2);
    REQUIRE(t.get(index_<2>) == 3);
    REQUIRE(t.get(index_<3>) == 4);
}

TEST_CASE("tuple_cat last empty", "[tuple]") {
    auto const t0 = cib::make_tuple(1, 2);
    auto const t1 = cib::make_tuple(3, 4);
    auto const t2 = cib::make_tuple();

    auto const t = tuple_cat(t0, t1, t2);

    REQUIRE(t.size() == 4);
    REQUIRE(t.get(index_<0>) == 1);
    REQUIRE(t.get(index_<1>) == 2);
    REQUIRE(t.get(index_<2>) == 3);
    REQUIRE(t.get(index_<3>) == 4);
}
