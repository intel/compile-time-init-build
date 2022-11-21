#include <cib/tuple.hpp>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <type_traits>

using cib::index_;
using cib::index_metafunc_;
using cib::tag_;

template <int Value> using int_t = std::integral_constant<int, Value>;

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

TEMPLATE_TEST_CASE("constructability", "[tuple]",
                   (cib::detail::tuple_element<int, 0>), cib::tuple<>,
                   (cib::tuple<A, int, bool>)) {
    static_assert(std::is_default_constructible_v<TestType>);
    static_assert(std::is_nothrow_default_constructible_v<TestType>);
}

TEMPLATE_TEST_CASE("copyability", "[tuple]",
                   (cib::detail::tuple_element<int, 0>), cib::tuple<>,
                   (cib::tuple<A, int, bool>)) {
    static_assert(std::is_copy_constructible_v<TestType>);
    static_assert(std::is_copy_assignable_v<TestType>);
    static_assert(std::is_nothrow_copy_constructible_v<TestType>);
    static_assert(std::is_nothrow_copy_assignable_v<TestType>);
    static_assert(std::is_trivially_copy_constructible_v<TestType>);
    static_assert(std::is_trivially_copy_assignable_v<TestType>);
}

TEMPLATE_TEST_CASE("moveability", "[tuple]",
                   (cib::detail::tuple_element<int, 0>), cib::tuple<>,
                   (cib::tuple<A, int, bool>)) {
    static_assert(std::is_move_constructible_v<TestType>);
    static_assert(std::is_move_assignable_v<TestType>);
    static_assert(std::is_nothrow_move_constructible_v<TestType>);
    static_assert(std::is_nothrow_move_assignable_v<TestType>);
    static_assert(std::is_trivially_move_constructible_v<TestType>);
    static_assert(std::is_trivially_move_assignable_v<TestType>);
}

TEMPLATE_TEST_CASE("destructability", "[tuple]",
                   (cib::detail::tuple_element<int, 0>), cib::tuple<>,
                   (cib::tuple<A, int, bool>)) {
    static_assert(std::is_nothrow_destructible_v<TestType>);
    static_assert(std::is_trivially_destructible_v<TestType>);
}

struct move_only {
    constexpr move_only() = default;
    constexpr move_only(int i) : value{i} {}
    constexpr move_only(move_only &&) = default;
    constexpr auto operator=(move_only &&) noexcept -> move_only & = default;

    friend constexpr auto operator==(move_only const &lhs, move_only const &rhs)
        -> bool {
        return lhs.value == rhs.value;
    }

    int value{};
};

TEST_CASE("move-only types", "[tuple]") {
    static_assert(std::is_default_constructible_v<cib::tuple<move_only>>);
    static_assert(
        std::is_nothrow_default_constructible_v<cib::tuple<move_only>>);

    static_assert(not std::is_copy_constructible_v<cib::tuple<move_only>>);
    static_assert(not std::is_copy_assignable_v<cib::tuple<move_only>>);

    static_assert(std::is_move_constructible_v<cib::tuple<move_only>>);
    static_assert(std::is_move_assignable_v<cib::tuple<move_only>>);
    static_assert(std::is_nothrow_move_constructible_v<cib::tuple<move_only>>);
    static_assert(std::is_nothrow_move_assignable_v<cib::tuple<move_only>>);
    static_assert(
        std::is_trivially_move_constructible_v<cib::tuple<move_only>>);
    static_assert(std::is_trivially_move_assignable_v<cib::tuple<move_only>>);

    static_assert(std::is_nothrow_destructible_v<cib::tuple<move_only>>);
    static_assert(std::is_trivially_destructible_v<cib::tuple<move_only>>);
}

TEST_CASE("empty make_tuple", "[tuple]") {
    constexpr auto t = cib::make_tuple();

    REQUIRE(t.size() == 0);
    static_assert(t.size() == 0);
}

TEST_CASE("unary make_tuple", "[tuple]") {
    constexpr auto t = cib::make_tuple(42);

    REQUIRE(t.size() == 1);
    REQUIRE(t.get(index_<0>) == 42);
    static_assert(t.size() == 1);
    static_assert(t.get(index_<0>) == 42);
}

TEST_CASE("n-ary make_tuple", "[tuple]") {
    constexpr auto t = cib::make_tuple(5, 10);

    REQUIRE(t.size() == 2);
    REQUIRE(t.get(index_<0>) == 5);
    REQUIRE(t.get(index_<1>) == 10);
    static_assert(t.size() == 2);
    static_assert(t.get(index_<0>) == 5);
    static_assert(t.get(index_<1>) == 10);
}

TEST_CASE("equality comparable", "[tuple]") {
    constexpr auto t = cib::make_tuple(5, 10);

    REQUIRE(t == t);
    REQUIRE(t != cib::make_tuple(5, 11));
    static_assert(t == t); // NOLINT(misc-redundant-expression)
    static_assert(t != cib::make_tuple(5, 11));
}

TEST_CASE("order comparable", "[tuple]") {
    constexpr auto t = cib::make_tuple(5, 10);

    REQUIRE(t < cib::make_tuple(6, 9));
    REQUIRE(t < cib::make_tuple(5, 11));
    REQUIRE(not(t < t));
    REQUIRE(not(t < cib::make_tuple(4, 11)));
    static_assert(t < cib::make_tuple(6, 9));
    static_assert(t < cib::make_tuple(5, 11));
    static_assert(not(t < t)); // NOLINT(misc-redundant-expression)
    static_assert(not(t < cib::make_tuple(4, 11)));

    REQUIRE(t <= t);
    REQUIRE(t <= cib::make_tuple(6, 9));
    REQUIRE(t <= cib::make_tuple(5, 11));
    REQUIRE(not(t <= cib::make_tuple(5, 9)));
    REQUIRE(not(t <= cib::make_tuple(4, 11)));
    static_assert(t <= t); // NOLINT(misc-redundant-expression)
    static_assert(t <= cib::make_tuple(6, 9));
    static_assert(t <= cib::make_tuple(5, 11));
    static_assert(not(t <= cib::make_tuple(5, 9)));
    static_assert(not(t <= cib::make_tuple(4, 11)));

    REQUIRE(t > cib::make_tuple(5, 9));
    REQUIRE(t > cib::make_tuple(4, 11));
    REQUIRE(not(t > t));
    REQUIRE(not(t > cib::make_tuple(6, 9)));
    static_assert(t > cib::make_tuple(5, 9));
    static_assert(t > cib::make_tuple(4, 11));
    static_assert(not(t > t)); // NOLINT(misc-redundant-expression)
    static_assert(not(t > cib::make_tuple(6, 9)));

    REQUIRE(t >= t);
    REQUIRE(t >= cib::make_tuple(5, 9));
    REQUIRE(t >= cib::make_tuple(4, 11));
    REQUIRE(not(t >= cib::make_tuple(5, 11)));
    REQUIRE(not(t >= cib::make_tuple(6, 9)));
    static_assert(t >= t); // NOLINT(misc-redundant-expression)
    static_assert(t >= cib::make_tuple(5, 9));
    static_assert(t >= cib::make_tuple(4, 11));
    static_assert(not(t >= cib::make_tuple(5, 11)));
    static_assert(not(t >= cib::make_tuple(6, 9)));
}

#if __cpp_lib_three_way_comparison >= 201907L
TEST_CASE("spaceship comparable", "[tuple]") {
    constexpr auto t = cib::make_tuple(5, 10);

    REQUIRE(t <=> t == std::strong_ordering::equal);
    REQUIRE(t <=> cib::make_tuple(6, 9) == std::strong_ordering::less);
    REQUIRE(t <=> cib::make_tuple(6, 10) == std::strong_ordering::less);
    REQUIRE(t <=> cib::make_tuple(5, 11) == std::strong_ordering::less);
    REQUIRE(t <=> cib::make_tuple(5, 9) == std::strong_ordering::greater);
    REQUIRE(t <=> cib::make_tuple(4, 10) == std::strong_ordering::greater);
    REQUIRE(t <=> cib::make_tuple(4, 11) == std::strong_ordering::greater);
    static_assert(t <=> t == std::strong_ordering::equal);
    static_assert(t <=> cib::make_tuple(6, 9) == std::strong_ordering::less);
    static_assert(t <=> cib::make_tuple(6, 10) == std::strong_ordering::less);
    static_assert(t <=> cib::make_tuple(5, 11) == std::strong_ordering::less);
    static_assert(t <=> cib::make_tuple(5, 9) == std::strong_ordering::greater);
    static_assert(t <=> cib::make_tuple(4, 10) ==
                  std::strong_ordering::greater);
    static_assert(t <=> cib::make_tuple(4, 11) ==
                  std::strong_ordering::greater);
}
#endif

TEST_CASE("self_type_index", "[tuple]") {
    constexpr auto t = cib::make_tuple(cib::self_type_index, 5, true, 10l);

    REQUIRE(t.size() == 3);
    REQUIRE(t.get(index_<0>) == 5);
    REQUIRE(t.get(index_<1>) == true);
    REQUIRE(t.get(index_<2>) == 10);
    static_assert(t.size() == 3);
    static_assert(t.get(index_<0>) == 5);
    static_assert(t.get(index_<1>) == true);
    static_assert(t.get(index_<2>) == 10);

    REQUIRE(t.get(tag_<int>) == 5);
    REQUIRE(t.get(tag_<bool>) == true);
    REQUIRE(t.get(tag_<long>) == 10);
    static_assert(t.get(tag_<int>) == 5);
    static_assert(t.get(tag_<bool>) == true);
    static_assert(t.get(tag_<long>) == 10);
}

template <typename KeyT, typename ValueT> struct map_entry {
    using Key = KeyT;
    using Value = ValueT;

    ValueT value;
};

struct extract_key {
    template <typename T> using invoke = typename T::Key;
};

TEST_CASE("empty make_tuple with calculated index", "[tuple]") {
    constexpr auto t = cib::make_tuple(index_metafunc_<extract_key>);

    REQUIRE(t.size() == 0);
    static_assert(t.size() == 0);
}

TEST_CASE("n-ary make_tuple with calculated index", "[tuple]") {
    constexpr auto t = cib::make_tuple(
        index_metafunc_<extract_key>, map_entry<A, int>{42},
        map_entry<B, int>{12}, map_entry<C, int>{55}, map_entry<D, int>{99});

    REQUIRE(t.size() == 4);
    static_assert(t.size() == 4);

    REQUIRE(t.get(index_<0>).value == 42);
    REQUIRE(t.get(index_<1>).value == 12);
    REQUIRE(t.get(index_<2>).value == 55);
    REQUIRE(t.get(index_<3>).value == 99);
    static_assert(t.get(index_<0>).value == 42);
    static_assert(t.get(index_<1>).value == 12);
    static_assert(t.get(index_<2>).value == 55);
    static_assert(t.get(index_<3>).value == 99);

    REQUIRE(t.get(tag_<A>).value == 42);
    REQUIRE(t.get(tag_<B>).value == 12);
    REQUIRE(t.get(tag_<C>).value == 55);
    REQUIRE(t.get(tag_<D>).value == 99);
    static_assert(t.get(tag_<A>).value == 42);
    static_assert(t.get(tag_<B>).value == 12);
    static_assert(t.get(tag_<C>).value == 55);
    static_assert(t.get(tag_<D>).value == 99);
}

TEST_CASE("tuple get value categories", "[tuple]") {
    {
        auto const t = cib::make_tuple(42);
        static_assert(std::is_same_v<decltype(t.get(index_<0>)), int const &>);
    }
    {
        auto t = cib::make_tuple(42);
        static_assert(std::is_same_v<decltype(t.get(index_<0>)), int &>);
    }
    static_assert(
        std::is_same_v<decltype(cib::make_tuple(42).get(index_<0>)), int &&>);

    {
        auto const t = cib::make_tuple(index_metafunc_<extract_key>,
                                       map_entry<A, int>{42});
        static_assert(std::is_same_v<decltype(t.get(tag_<A>)),
                                     map_entry<A, int> const &>);
    }
    {
        auto t = cib::make_tuple(index_metafunc_<extract_key>,
                                 map_entry<A, int>{42});
        static_assert(
            std::is_same_v<decltype(t.get(tag_<A>)), map_entry<A, int> &>);
    }
    static_assert(
        std::is_same_v<decltype(cib::make_tuple(index_metafunc_<extract_key>,
                                                map_entry<A, int>{42})
                                    .get(tag_<A>)),
                       map_entry<A, int> &&>);
}

TEST_CASE("tuple_cat", "[tuple]") {
    REQUIRE(tuple_cat(cib::make_tuple(5, 10), cib::make_tuple(12, 30)) ==
            cib::make_tuple(5, 10, 12, 30));
    static_assert(tuple_cat(cib::make_tuple(5, 10), cib::make_tuple(12, 30)) ==
                  cib::make_tuple(5, 10, 12, 30));
}

TEST_CASE("tuple_cat left empty", "[tuple]") {
    REQUIRE(tuple_cat(cib::make_tuple(), cib::make_tuple(12, 30)) ==
            cib::make_tuple(12, 30));
    static_assert(tuple_cat(cib::make_tuple(), cib::make_tuple(12, 30)) ==
                  cib::make_tuple(12, 30));
}

TEST_CASE("tuple_cat right empty", "[tuple]") {
    REQUIRE(tuple_cat(cib::make_tuple(5, 10), cib::make_tuple()) ==
            cib::make_tuple(5, 10));
    static_assert(tuple_cat(cib::make_tuple(5, 10), cib::make_tuple()) ==
                  cib::make_tuple(5, 10));
}

TEST_CASE("tuple_cat both empty", "[tuple]") {
    REQUIRE(tuple_cat(cib::make_tuple(), cib::make_tuple()) ==
            cib::make_tuple());
    static_assert(tuple_cat(cib::make_tuple(), cib::make_tuple()) ==
                  cib::make_tuple());
}

TEST_CASE("tuple_cat three empty", "[tuple]") {
    REQUIRE(tuple_cat(cib::make_tuple(), cib::make_tuple(),
                      cib::make_tuple()) == cib::make_tuple());
    static_assert(tuple_cat(cib::make_tuple(), cib::make_tuple(),
                            cib::make_tuple()) == cib::make_tuple());
}

TEST_CASE("tuple_cat middle empty", "[tuple]") {
    REQUIRE(tuple_cat(cib::make_tuple(1, 2), cib::make_tuple(),
                      cib::make_tuple(3, 4)) == cib::make_tuple(1, 2, 3, 4));
    static_assert(tuple_cat(cib::make_tuple(1, 2), cib::make_tuple(),
                            cib::make_tuple(3, 4)) ==
                  cib::make_tuple(1, 2, 3, 4));
}

TEST_CASE("tuple_cat first empty", "[tuple]") {
    REQUIRE(tuple_cat(cib::make_tuple(), cib::make_tuple(1, 2),
                      cib::make_tuple(3, 4)) == cib::make_tuple(1, 2, 3, 4));
    static_assert(tuple_cat(cib::make_tuple(), cib::make_tuple(1, 2),
                            cib::make_tuple(3, 4)) ==
                  cib::make_tuple(1, 2, 3, 4));
}

TEST_CASE("tuple_cat last empty", "[tuple]") {
    REQUIRE(tuple_cat(cib::make_tuple(1, 2), cib::make_tuple(3, 4),
                      cib::make_tuple()) == cib::make_tuple(1, 2, 3, 4));
    static_assert(tuple_cat(cib::make_tuple(1, 2), cib::make_tuple(3, 4),
                            cib::make_tuple()) == cib::make_tuple(1, 2, 3, 4));
}

TEST_CASE("tuple_cat (move only)", "[tuple]") {
    REQUIRE(tuple_cat(cib::make_tuple(move_only{5}),
                      cib::make_tuple(move_only{10})) ==
            cib::make_tuple(move_only{5}, move_only{10}));
    static_assert(tuple_cat(cib::make_tuple(move_only{5}),
                            cib::make_tuple(move_only{10})) ==
                  cib::make_tuple(move_only{5}, move_only{10}));
}

template <typename KeyT0, typename KeyT1, typename KeyT2, typename ValueT>
struct multi_map_entry {
    using Key0 = KeyT0;
    using Key1 = KeyT1;
    using Key2 = KeyT2;
    using Value = ValueT;

    ValueT value;
};

struct extract_key_0 {
    template <typename T> using invoke = typename T::Key0;
};

struct extract_key_1 {
    template <typename T> using invoke = typename T::Key1;
};

struct extract_key_2 {
    template <typename T> using invoke = typename T::Key2;
};

TEST_CASE("make_tuple with multiple calculated index", "[tuple]") {
    constexpr auto t = cib::make_tuple(
        index_metafunc_<extract_key_0>, index_metafunc_<extract_key_1>,
        index_metafunc_<extract_key_2>, multi_map_entry<A, B, C, int>{42},
        multi_map_entry<D, E, F, int>{12}, multi_map_entry<G, H, I, int>{55},
        multi_map_entry<J, K, L, int>{99});

    REQUIRE(t.size() == 4);
    static_assert(t.size() == 4);

    REQUIRE(t.get(index_<0>).value == 42);
    REQUIRE(t.get(index_<1>).value == 12);
    REQUIRE(t.get(index_<2>).value == 55);
    REQUIRE(t.get(index_<3>).value == 99);
    static_assert(t.get(index_<0>).value == 42);
    static_assert(t.get(index_<1>).value == 12);
    static_assert(t.get(index_<2>).value == 55);
    static_assert(t.get(index_<3>).value == 99);

    REQUIRE(t.get(tag_<A>).value == 42);
    REQUIRE(t.get(tag_<B>).value == 42);
    REQUIRE(t.get(tag_<C>).value == 42);
    static_assert(t.get(tag_<A>).value == 42);
    static_assert(t.get(tag_<B>).value == 42);
    static_assert(t.get(tag_<C>).value == 42);

    REQUIRE(t.get(tag_<D>).value == 12);
    REQUIRE(t.get(tag_<E>).value == 12);
    REQUIRE(t.get(tag_<F>).value == 12);
    static_assert(t.get(tag_<D>).value == 12);
    static_assert(t.get(tag_<E>).value == 12);
    static_assert(t.get(tag_<F>).value == 12);

    REQUIRE(t.get(tag_<G>).value == 55);
    REQUIRE(t.get(tag_<H>).value == 55);
    REQUIRE(t.get(tag_<I>).value == 55);
    static_assert(t.get(tag_<G>).value == 55);
    static_assert(t.get(tag_<H>).value == 55);
    static_assert(t.get(tag_<I>).value == 55);

    REQUIRE(t.get(tag_<J>).value == 99);
    REQUIRE(t.get(tag_<K>).value == 99);
    REQUIRE(t.get(tag_<L>).value == 99);
    static_assert(t.get(tag_<J>).value == 99);
    static_assert(t.get(tag_<K>).value == 99);
    static_assert(t.get(tag_<L>).value == 99);
}

TEST_CASE("apply", "[tuple]") {
    REQUIRE(cib::apply([](auto... xs) { return (0 + ... + xs); },
                       cib::make_tuple(1, 2, 3)) == 6);
    static_assert(cib::apply([](auto... xs) { return (0 + ... + xs); },
                             cib::make_tuple(1, 2, 3)) == 6);

    auto stateful = [calls = 0](auto...) mutable { return ++calls; };
    REQUIRE(cib::apply(stateful, cib::make_tuple(1, 2, 3)) == 1);
    REQUIRE(cib::apply(stateful, cib::make_tuple(1, 2, 3)) == 2);

    REQUIRE(cib::apply([](auto x) { return x.value; },
                       cib::make_tuple(move_only{42})) == 42);
    static_assert(cib::apply([](auto x) { return x.value; },
                             cib::make_tuple(move_only{42})) == 42);
}

TEST_CASE("fold_left", "[tuple]") {
    constexpr auto t = cib::make_tuple(1, 2, 3);
    REQUIRE(t.fold_left(0, std::minus{}) == (((0 - 1) - 2) - 3));
    static_assert(t.fold_left(0, std::minus{}) == (((0 - 1) - 2) - 3));

    REQUIRE(cib::make_tuple(move_only{1}, move_only{2}, move_only{3})
                .fold_left(move_only{0},
                           [](move_only &&x, move_only &&y) {
                               return move_only{x.value + y.value};
                           })
                .value == 6);
    static_assert(cib::make_tuple(move_only{1}, move_only{2}, move_only{3})
                      .fold_left(move_only{0},
                                 [](move_only &&x, move_only &&y) {
                                     return move_only{x.value + y.value};
                                 })
                      .value == 6);

    int calls{};
    auto stateful = [&](auto x, auto y) mutable {
        ++calls;
        return x + y;
    };
    REQUIRE(t.fold_left(0, stateful) == 6);
    REQUIRE(calls == 3);
}

TEST_CASE("fold_left (no initial value)", "[tuple]") {
    constexpr auto t = cib::make_tuple(1, 2, 3);
    REQUIRE(t.fold_left(std::minus{}) == ((1 - 2) - 3));
    static_assert(t.fold_left(std::minus{}) == ((1 - 2) - 3));

    REQUIRE(cib::make_tuple(move_only{1}, move_only{2}, move_only{3})
                .fold_left([](move_only &&x, move_only &&y) {
                    return move_only{x.value + y.value};
                })
                .value == 6);
    static_assert(cib::make_tuple(move_only{1}, move_only{2}, move_only{3})
                      .fold_left([](move_only &&x, move_only &&y) {
                          return move_only{x.value + y.value};
                      })
                      .value == 6);

    int calls{};
    auto stateful = [&](auto x, auto y) mutable {
        ++calls;
        return x + y;
    };
    REQUIRE(t.fold_left(stateful) == 6);
    REQUIRE(calls == 2);
}

TEST_CASE("fold_left (heterogeneous types in fold)", "[tuple]") {
    constexpr auto t = cib::make_tuple(1, 2, 3);
    REQUIRE(t.fold_left(cib::make_tuple(), [](auto acc, auto n) {
        return cib::tuple_cat(acc, cib::make_tuple(n));
    }) == t);
    static_assert(t.fold_left(cib::make_tuple(), [](auto acc, auto n) {
        return cib::tuple_cat(acc, cib::make_tuple(n));
    }) == t);

    REQUIRE(cib::make_tuple(1, 2, 3).fold_left(
                cib::make_tuple(), [](auto acc, auto n) {
                    return cib::tuple_cat(acc, cib::make_tuple(n));
                }) == t);
    static_assert(cib::make_tuple(1, 2, 3).fold_left(
                      cib::make_tuple(), [](auto acc, auto n) {
                          return cib::tuple_cat(acc, cib::make_tuple(n));
                      }) == t);

    static_assert(
        cib::make_tuple().fold_left(cib::make_tuple(), [](auto acc, auto n) {
            return cib::tuple_cat(acc, cib::make_tuple(n));
        }) == cib::make_tuple());
}

template <auto N> struct addend {
    constexpr friend auto operator==(addend, addend) -> bool { return true; }
};
template <auto X, auto Y> constexpr auto operator+(addend<X>, addend<Y>) {
    return addend<X + Y>{};
}

TEST_CASE("fold_left (heterogeneous types in tuple)", "[tuple]") {
    constexpr auto t = cib::make_tuple(addend<1>{}, addend<2>{});
    REQUIRE(t.fold_left(addend<0>{}, std::plus{}) == addend<3>{});
    static_assert(t.fold_left(addend<0>{}, std::plus{}) == addend<3>{});
    REQUIRE(t.fold_left(std::plus{}) == addend<3>{});
    static_assert(t.fold_left(std::plus{}) == addend<3>{});
}

TEST_CASE("fold_right", "[tuple]") {
    constexpr auto t = cib::make_tuple(1, 2, 3);
    REQUIRE(t.fold_right(4, std::minus{}) == (1 - (2 - (3 - 4))));
    static_assert(t.fold_right(4, std::minus{}) == (1 - (2 - (3 - 4))));

    REQUIRE(cib::make_tuple(move_only{1}, move_only{2}, move_only{3})
                .fold_right(move_only{0},
                            [](move_only &&x, move_only &&y) {
                                return move_only{x.value + y.value};
                            })
                .value == 6);
    static_assert(cib::make_tuple(move_only{1}, move_only{2}, move_only{3})
                      .fold_right(move_only{0},
                                  [](move_only &&x, move_only &&y) {
                                      return move_only{x.value + y.value};
                                  })
                      .value == 6);

    int calls{};
    auto stateful = [&](auto x, auto y) mutable {
        ++calls;
        return x + y;
    };
    REQUIRE(t.fold_right(0, stateful) == 6);
    REQUIRE(calls == 3);
}

TEST_CASE("fold_right (no initial value)", "[tuple]") {
    constexpr auto t = cib::make_tuple(1, 2, 3);
    REQUIRE(t.fold_right(std::minus{}) == (1 - (2 - 3)));
    static_assert(t.fold_right(std::minus{}) == (1 - (2 - 3)));

    REQUIRE(cib::make_tuple(move_only{1}, move_only{2}, move_only{3})
                .fold_right([](move_only &&x, move_only &&y) {
                    return move_only{x.value + y.value};
                })
                .value == 6);
    static_assert(cib::make_tuple(move_only{1}, move_only{2}, move_only{3})
                      .fold_right([](move_only &&x, move_only &&y) {
                          return move_only{x.value + y.value};
                      })
                      .value == 6);

    int calls{};
    auto stateful = [&](auto x, auto y) mutable {
        ++calls;
        return x + y;
    };
    REQUIRE(t.fold_right(stateful) == 6);
    REQUIRE(calls == 2);
}

TEST_CASE("fold_right (heterogeneous types)", "[tuple]") {
    constexpr auto t = cib::make_tuple(1, 2, 3);
    REQUIRE(t.fold_right(cib::make_tuple(), [](auto n, auto acc) {
        return cib::tuple_cat(cib::make_tuple(n), acc);
    }) == t);
    static_assert(t.fold_right(cib::make_tuple(), [](auto n, auto acc) {
        return cib::tuple_cat(cib::make_tuple(n), acc);
    }) == t);

    REQUIRE(cib::make_tuple(1, 2, 3).fold_right(
                cib::make_tuple(), [](auto n, auto acc) {
                    return cib::tuple_cat(cib::make_tuple(n), acc);
                }) == t);
    static_assert(cib::make_tuple(1, 2, 3).fold_right(
                      cib::make_tuple(), [](auto n, auto acc) {
                          return cib::tuple_cat(cib::make_tuple(n), acc);
                      }) == t);

    static_assert(
        cib::make_tuple().fold_right(cib::make_tuple(), [](auto n, auto acc) {
            return cib::tuple_cat(cib::make_tuple(n), acc);
        }) == cib::make_tuple());
}

TEST_CASE("fold_right (heterogeneous types in tuple)", "[tuple]") {
    constexpr auto t = cib::make_tuple(addend<1>{}, addend<2>{});
    REQUIRE(t.fold_right(addend<0>{}, std::plus{}) == addend<3>{});
    static_assert(t.fold_right(addend<0>{}, std::plus{}) == addend<3>{});
    REQUIRE(t.fold_right(std::plus{}) == addend<3>{});
    static_assert(t.fold_right(std::plus{}) == addend<3>{});
}

TEST_CASE("for_each (member function)", "[tuple]") {
    {
        auto const t = cib::make_tuple(1, 2, 3);
        auto sum = 0;
        t.for_each([&](auto x) { sum += x; });
        REQUIRE(sum == 6);
    }
    {
        auto t = cib::make_tuple(1, 2, 3);
        t.for_each([](auto &x) { x += 5; });
        REQUIRE(t == cib::make_tuple(6, 7, 8));
    }
    {
        auto const t = cib::make_tuple(1, 2, 3);
        auto f = t.for_each([calls = 0](auto) mutable {
            ++calls;
            return calls;
        });
        REQUIRE(f(0) == 4);
    }
}

TEST_CASE("for_each (free function)", "[tuple]") {
    {
        const auto t = cib::make_tuple(1, 2, 3);
        auto sum = 0;
        for_each([&](auto x, auto y) { sum += x + y; }, t, t);
        REQUIRE(sum == 12);
    }
    {
        const auto t = cib::make_tuple(1);
        auto sum = 0;
        for_each([&](auto x, auto &&y) { sum += x + y.value; }, t,
                 cib::make_tuple(move_only{2}));
        REQUIRE(sum == 3);
    }
    {
        auto const t = cib::make_tuple(1, 2, 3);
        auto f = for_each(
            [calls = 0](auto) mutable {
                ++calls;
                return calls;
            },
            t);
        REQUIRE(f(0) == 4);
    }
}

TEST_CASE("unary transform (without indexing)", "[tuple]") {
    constexpr auto t = cib::make_tuple(1, 2, 3);
    constexpr auto u = transform([](auto x) { return x + 1; }, t);
    REQUIRE(u == cib::make_tuple(2, 3, 4));
    static_assert(u == cib::make_tuple(2, 3, 4));
}

TEST_CASE("n-ary transform (without indexing)", "[tuple]") {
    constexpr auto t = cib::make_tuple(1, 2, 3);
    constexpr auto u = transform([](auto x, auto y) { return x + y; }, t, t);
    REQUIRE(u == cib::make_tuple(2, 4, 6));
    static_assert(u == cib::make_tuple(2, 4, 6));
}

TEST_CASE("unary transform (with indexing)", "[tuple]") {
    constexpr auto t = cib::make_tuple(5, true, 10l);
    constexpr auto u =
        cib::transform<cib::self_type>([](auto x) { return x; }, t);
    REQUIRE(u == cib::make_tuple(cib::self_type_index, 5, true, 10l));
    static_assert(u == cib::make_tuple(cib::self_type_index, 5, true, 10l));
}

TEST_CASE("n-ary transform (with indexing)", "[tuple]") {
    constexpr auto t = cib::make_tuple(5, true, 10l);
    constexpr auto u =
        cib::transform<cib::self_type>([](auto x, auto) { return x; }, t, t);
    REQUIRE(u == cib::make_tuple(cib::self_type_index, 5, true, 10l));
    static_assert(u == cib::make_tuple(cib::self_type_index, 5, true, 10l));
}

TEST_CASE("filter", "[tuple]") {
    constexpr auto t = cib::make_tuple(
        std::integral_constant<int, 1>{}, std::integral_constant<int, 2>{},
        std::integral_constant<int, 3>{}, std::integral_constant<int, 4>{});
    constexpr auto u = filter(t, [](auto x) { return x % 2 == 0; });
    REQUIRE(u == cib::make_tuple(std::integral_constant<int, 2>{},
                                 std::integral_constant<int, 4>{}));
    static_assert(u == cib::make_tuple(std::integral_constant<int, 2>{},
                                       std::integral_constant<int, 4>{}));
}
