#include <cib/cib.hpp>

#include <catch2/catch_test_macros.hpp>

using cib::detail::tag_;
using cib::detail::index_;
using cib::detail::index_metafunc_;
using cib::detail::tuple_element;
using cib::detail::tuple;
using cib::detail::make_tuple;

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

TEST_CASE("make empty tuple", "[tuple]") {
    auto const t = tuple<>{};
    REQUIRE(t.size() == 0);
}

TEST_CASE("make small tuple", "[tuple]") {
    auto const t =
        tuple{
            tuple_element<int, 0, A>{5},
            tuple_element<int, 1, B>{10}
        };

    REQUIRE(t.size() == 2);
    REQUIRE(t.get(index_<0>) == 5);
    REQUIRE(t.get(index_<1>) == 10);
    REQUIRE(t.get(tag_<A>) == 5);
    REQUIRE(t.get(tag_<B>) == 10);
}

struct extract_key {
    template<typename T>
    using invoke = typename T::Key;
};

TEST_CASE("make_tuple with calculated index", "[tuple]") {
    auto const t =
        make_tuple(
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

TEST_CASE("simple make_tuple", "[tuple]") {
    auto const t = make_tuple(5, 10);

    REQUIRE(t.size() == 2);
    REQUIRE(t.get(index_<0>) == 5);
    REQUIRE(t.get(index_<1>) == 10);
}

TEST_CASE("indexed_tuple_cat", "[tuple]") {
    auto const t0 = make_tuple(5, 10);
    auto const t1 = make_tuple(12, 30);

    auto const t = indexed_tuple_cat(t0, t1);

    REQUIRE(t.size() == 4);
    REQUIRE(t.get(index_<0>) == 5);
    REQUIRE(t.get(index_<1>) == 10);
    REQUIRE(t.get(index_<2>) == 12);
    REQUIRE(t.get(index_<3>) == 30);
}
