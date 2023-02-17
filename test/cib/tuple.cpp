#include "special_members.hpp"

#include <cib/tuple.hpp>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <type_traits>
#include <utility>

TEST_CASE("empty tuple", "[tuple]") {
    static_assert(std::is_empty_v<cib::tuple<>>);
    constexpr auto t = cib::tuple{};
    using T = std::remove_const_t<decltype(t)>;
    static_assert(std::is_same_v<T, cib::tuple<>>);
    static_assert(cib::tuple_size_v<T> == 0);
    static_assert(T::size() == 0);
}

TEST_CASE("single element tuple", "[tuple]") {
    constexpr auto t = cib::tuple{1};
    using T = std::remove_const_t<decltype(t)>;
    static_assert(std::is_same_v<T, cib::tuple<int>>);
    static_assert(cib::tuple_size_v<T> == 1);
    static_assert(T::size() == 1);
    static_assert(sizeof(T) == sizeof(int));

    auto x = 1;
    auto u = cib::tuple{x};
    using U = decltype(u);
    static_assert(std::is_same_v<U, cib::tuple<int>>);
    static_assert(cib::tuple_size_v<U> == 1);
    static_assert(U::size() == 1);
    static_assert(sizeof(U) == sizeof(int));
}

TEST_CASE("multi element tuple", "[tuple]") {
    constexpr auto t = cib::tuple{1, 2.0f};
    using T = std::remove_const_t<decltype(t)>;
    static_assert(std::is_same_v<T, cib::tuple<int, float>>);
    static_assert(cib::tuple_size_v<T> == 2);
    static_assert(T::size() == 2);
}

TEST_CASE("constexpr tuple of references", "[tuple]") {
    static constexpr int x = 1;
    constexpr auto t = cib::tuple<int const &>{x};
    using T = std::remove_const_t<decltype(t)>;
    static_assert(cib::tuple_size_v<T> == 1);
    static_assert(T::size() == 1);
}

TEST_CASE("free get", "[tuple]") {
    constexpr auto t = cib::tuple{5, true, 10l};

    REQUIRE(cib::get<0>(t) == 5);
    REQUIRE(cib::get<1>(t) == true);
    REQUIRE(cib::get<2>(t) == 10);
    static_assert(cib::get<0>(t) == 5);
    static_assert(cib::get<1>(t) == true);
    static_assert(cib::get<2>(t) == 10);

    REQUIRE(cib::get<int>(t) == 5);
    REQUIRE(cib::get<bool>(t) == true);
    REQUIRE(cib::get<long>(t) == 10);
    static_assert(cib::get<int>(t) == 5);
    static_assert(cib::get<bool>(t) == true);
    static_assert(cib::get<long>(t) == 10);
}

TEST_CASE("free get value categories", "[tuple]") {
    {
        auto const t = cib::tuple{42};
        static_assert(std::is_same_v<decltype(cib::get<0>(t)), int const &>);
        static_assert(std::is_same_v<decltype(cib::get<int>(t)), int const &>);
    }
    {
        auto t = cib::tuple{42};
        static_assert(std::is_same_v<decltype(cib::get<0>(t)), int &>);
        static_assert(std::is_same_v<decltype(cib::get<int>(t)), int &>);
        static_assert(
            std::is_same_v<decltype(cib::get<0>(std::move(t))), int &&>);
        static_assert(
            std::is_same_v<decltype(cib::get<int>(std::move(t))), int &&>);
    }
}

TEST_CASE("indexing", "[tuple]") {
    using namespace cib::tuple_literals;

    constexpr auto t = cib::tuple{5, true, 10l};
    static_assert(t[0_idx] == 5);
    static_assert(t[1_idx] == true);
    static_assert(t[2_idx] == 10l);

    static_assert(std::is_same_v<decltype(t[0_idx]), int const &>);
    auto u = cib::tuple{1};
    static_assert(std::is_same_v<decltype(u[0_idx]), int &>);
    static_assert(std::is_same_v<decltype(cib::tuple{1}[0_idx]), int &&>);
}

TEST_CASE("tuple of lvalue references", "[tuple]") {
    auto x = 1;
    auto t = cib::tuple<int &>{x};
    CHECK(cib::get<0>(t) == 1);
    cib::get<0>(t) = 2;
    CHECK(cib::get<0>(t) == 2);
    CHECK(x == 2);
}

TEST_CASE("tuple of lambdas", "[tuple]") {
    auto x = 1;
    auto t = cib::make_tuple([&] { x = 2; }, [&] { x = 3; });
    cib::get<0>(t)();
    CHECK(x == 2);
}

TEST_CASE("tuple size/elements", "[tuple]") {
    using T = cib::tuple<int, bool>;
    static_assert(cib::tuple_size_v<T> == 2);
    static_assert(std::is_same_v<cib::tuple_element_t<0, T>, int>);
    static_assert(std::is_same_v<cib::tuple_element_t<1, T>, bool>);

    using A = cib::tuple<int &>;
    static_assert(std::is_same_v<cib::tuple_element_t<0, A>, int &>);
    using B = cib::tuple<int const &>;
    static_assert(std::is_same_v<cib::tuple_element_t<0, B>, int const &>);
    using C = cib::tuple<int &&>;
    static_assert(std::is_same_v<cib::tuple_element_t<0, C>, int &&>);
}

namespace {
struct A {};
struct B {
    B(int) {}
};
} // namespace

TEST_CASE("default constructability", "[tuple]") {
    static_assert(std::is_default_constructible_v<cib::tuple<A>>);
    static_assert(std::is_nothrow_default_constructible_v<cib::tuple<A>>);
    static_assert(not std::is_default_constructible_v<cib::tuple<B>>);
    static_assert(not std::is_nothrow_default_constructible_v<cib::tuple<B>>);
}

TEMPLATE_TEST_CASE("constructability", "[tuple]",
                   (cib::detail::element<0, int>), cib::tuple<>,
                   (cib::tuple<A, int, bool>)) {
    static_assert(std::is_default_constructible_v<TestType>);
    static_assert(std::is_nothrow_default_constructible_v<TestType>);
}

TEMPLATE_TEST_CASE("copyability", "[tuple]", (cib::detail::element<0, int>),
                   cib::tuple<>, (cib::tuple<A, int, bool>)) {
    static_assert(std::is_copy_constructible_v<TestType>);
    static_assert(std::is_copy_assignable_v<TestType>);
    static_assert(std::is_nothrow_copy_constructible_v<TestType>);
    static_assert(std::is_nothrow_copy_assignable_v<TestType>);
    static_assert(std::is_trivially_copy_constructible_v<TestType>);
    static_assert(std::is_trivially_copy_assignable_v<TestType>);
}

TEMPLATE_TEST_CASE("moveability", "[tuple]", (cib::detail::element<0, int>),
                   cib::tuple<>, (cib::tuple<A, int, bool>)) {
    static_assert(std::is_move_constructible_v<TestType>);
    static_assert(std::is_move_assignable_v<TestType>);
    static_assert(std::is_nothrow_move_constructible_v<TestType>);
    static_assert(std::is_nothrow_move_assignable_v<TestType>);
    static_assert(std::is_trivially_move_constructible_v<TestType>);
    static_assert(std::is_trivially_move_assignable_v<TestType>);
}

TEMPLATE_TEST_CASE("destructability", "[tuple]", (cib::detail::element<0, int>),
                   cib::tuple<>, (cib::tuple<A, int, bool>)) {
    static_assert(std::is_nothrow_destructible_v<TestType>);
    static_assert(std::is_trivially_destructible_v<TestType>);
}

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

TEST_CASE("equality comparable", "[tuple]") {
    constexpr auto t = cib::tuple{5, 10};

    REQUIRE(t == t);
    REQUIRE(t != cib::tuple{5, 11});
    static_assert(t == t); // NOLINT(misc-redundant-expression)
    static_assert(t != cib::tuple{5, 11});
}

namespace {
struct eq {
    [[nodiscard]] friend constexpr auto operator==(eq lhs, eq rhs) -> bool {
        return lhs.x == rhs.x;
    }
    int x;
};
} // namespace

TEST_CASE("equality comparable (user-defined)", "[tuple]") {
    constexpr auto t = cib::tuple{eq{1}};

    static_assert(t == cib::tuple{eq{1}});
    static_assert(t != cib::tuple{eq{2}});
}

TEST_CASE("order comparable", "[tuple]") {
    constexpr auto t = cib::tuple{5, 10};

    REQUIRE(t < cib::tuple{6, 9});
    REQUIRE(t < cib::tuple{5, 11});
    REQUIRE(not(t < t));
    REQUIRE(not(t < cib::tuple{4, 11}));
    static_assert(t < cib::tuple{6, 9});
    static_assert(t < cib::tuple{5, 11});
    static_assert(not(t < t)); // NOLINT(misc-redundant-expression)
    static_assert(not(t < cib::tuple{4, 11}));

    REQUIRE(t <= t);
    REQUIRE(t <= cib::tuple{6, 9});
    REQUIRE(t <= cib::tuple{5, 11});
    REQUIRE(not(t <= cib::tuple{5, 9}));
    REQUIRE(not(t <= cib::tuple{4, 11}));
    static_assert(t <= t); // NOLINT(misc-redundant-expression)
    static_assert(t <= cib::tuple{6, 9});
    static_assert(t <= cib::tuple{5, 11});
    static_assert(not(t <= cib::tuple{5, 9}));
    static_assert(not(t <= cib::tuple{4, 11}));

    REQUIRE(t > cib::tuple{5, 9});
    REQUIRE(t > cib::tuple{4, 11});
    REQUIRE(not(t > t));
    REQUIRE(not(t > cib::tuple{6, 9}));
    static_assert(t > cib::tuple{5, 9});
    static_assert(t > cib::tuple{4, 11});
    static_assert(not(t > t)); // NOLINT(misc-redundant-expression)
    static_assert(not(t > cib::tuple{6, 9}));

    REQUIRE(t >= t);
    REQUIRE(t >= cib::tuple{5, 9});
    REQUIRE(t >= cib::tuple{4, 11});
    REQUIRE(not(t >= cib::tuple{5, 11}));
    REQUIRE(not(t >= cib::tuple{6, 9}));
    static_assert(t >= t); // NOLINT(misc-redundant-expression)
    static_assert(t >= cib::tuple{5, 9});
    static_assert(t >= cib::tuple{4, 11});
    static_assert(not(t >= cib::tuple{5, 11}));
    static_assert(not(t >= cib::tuple{6, 9}));
}

TEST_CASE("spaceship comparable", "[tuple]") {
    constexpr auto t = cib::tuple{5, 10};

    REQUIRE(t <=> t == std::strong_ordering::equal);
    REQUIRE(t <=> cib::tuple{6, 9} == std::strong_ordering::less);
    REQUIRE(t <=> cib::tuple{6, 10} == std::strong_ordering::less);
    REQUIRE(t <=> cib::tuple{5, 11} == std::strong_ordering::less);
    REQUIRE(t <=> cib::tuple{5, 9} == std::strong_ordering::greater);
    REQUIRE(t <=> cib::tuple{4, 10} == std::strong_ordering::greater);
    REQUIRE(t <=> cib::tuple{4, 11} == std::strong_ordering::greater);
    static_assert(t <=> t == std::strong_ordering::equal);
    static_assert(t <=> cib::tuple{6, 9} == std::strong_ordering::less);
    static_assert(t <=> cib::tuple{6, 10} == std::strong_ordering::less);
    static_assert(t <=> cib::tuple{5, 11} == std::strong_ordering::less);
    static_assert(t <=> cib::tuple{5, 9} == std::strong_ordering::greater);
    static_assert(t <=> cib::tuple{4, 10} == std::strong_ordering::greater);
    static_assert(t <=> cib::tuple{4, 11} == std::strong_ordering::greater);
}

TEST_CASE("free get is SFINAE-friendly", "[tuple]") {
    constexpr auto t = []<typename... Ts>(cib::tuple<Ts...> const &tup) {
        return cib::tuple{cib::get<Ts>(tup)...};
    }(cib::tuple{});
    static_assert(t == cib::tuple{});
}

TEST_CASE("copy/move behavior for tuple", "[tuple]") {
    counter::reset();
    auto t1 = cib::tuple{counter{}};
    auto const orig_moves = counter::moves;
    auto const orig_copies = counter::copies;

    [[maybe_unused]] auto t2 = t1;
    CHECK(counter::moves == orig_moves);
    CHECK(counter::copies == orig_copies + 1);

    [[maybe_unused]] auto t3 = std::move(t1);
    CHECK(counter::moves == orig_moves + 1);
    CHECK(counter::copies == orig_copies + 1);
}

namespace {
template <typename Key, typename Value> struct map_entry {
    using key_t = Key;
    using value_t = Value;

    value_t value;
};
template <typename T> using key_for = typename T::key_t;
} // namespace

TEST_CASE("make_tuple", "[tuple]") {
    static_assert(cib::make_tuple() == cib::tuple{});
    static_assert(cib::make_tuple(1, 2, 3) == cib::tuple{1, 2, 3});

    constexpr auto t = cib::make_tuple(cib::tuple{});
    using T = std::remove_const_t<decltype(t)>;
    static_assert(std::is_same_v<T, cib::tuple<cib::tuple<>>>);
    static_assert(cib::tuple_size_v<T> == 1);
    static_assert(T::size() == 1);
}

TEST_CASE("make_indexed_tuple", "[tuple]") {
    static_assert(cib::make_indexed_tuple<>() == cib::tuple{});
    static_assert(cib::make_indexed_tuple<>(1, 2, 3) == cib::tuple{1, 2, 3});
}

TEST_CASE("tuple with user index", "[tuple]") {
    struct X;
    struct Y;
    constexpr auto t = cib::make_indexed_tuple<key_for>(map_entry<X, int>{42},
                                                        map_entry<Y, int>{17});
    static_assert(cib::get<X>(t).value == 42);
    static_assert(cib::get<Y>(t).value == 17);
    using T = std::remove_const_t<decltype(t)>;
    static_assert(
        std::is_same_v<
            T, cib::indexed_tuple<cib::detail::index_function_list<key_for>,
                                  map_entry<X, int>, map_entry<Y, int>>>);
    static_assert(cib::tuple_size_v<T> == 2);
    static_assert(T::size() == 2);
}

namespace {
template <typename Key1, typename Key2, typename Value> struct multimap_entry {
    using key1_t = Key1;
    using key2_t = Key2;
    using value_t = Value;

    value_t value;
};

template <typename T> using key1_for = typename T::key1_t;
template <typename T> using key2_for = typename T::key2_t;
} // namespace

TEST_CASE("tuple with multiple user indices", "[tuple]") {
    struct M;
    struct N;
    struct X;
    struct Y;
    constexpr auto t = cib::make_indexed_tuple<key1_for, key2_for>(
        multimap_entry<M, X, int>{42}, multimap_entry<N, Y, int>{17});
    static_assert(cib::get<M>(t).value == 42);
    static_assert(cib::get<X>(t).value == 42);
    static_assert(cib::get<N>(t).value == 17);
    static_assert(cib::get<Y>(t).value == 17);
}

TEST_CASE("apply indices", "[tuple]") {
    struct X;
    constexpr auto t = cib::tuple{map_entry<X, int>{42}};
    constexpr auto u = cib::apply_indices<key_for>(t);
    static_assert(cib::get<X>(u).value == 42);
}
