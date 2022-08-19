#include <catch2/catch_test_macros.hpp>

#include <container/ConstexprMultiMap.hpp>

namespace {
    TEST_CASE("EmptyAndSize", "[constexpr_multimap]") {
        ConstexprMultiMap<int, int, 64> t;

        REQUIRE(t.getSize() == 0);
        REQUIRE(t.isEmpty() == true);
    }

    TEST_CASE("PutAndContains", "[constexpr_multimap]") {
        ConstexprMultiMap<int, int, 64> t;

        t.put(60, 40);

        REQUIRE(t.getSize() == 1);
        REQUIRE(t.isEmpty() == false);
        REQUIRE(t.contains(60) == true);
        REQUIRE(t.contains(40) == false);
        REQUIRE(t.contains(60, 40) == true);
        REQUIRE(t.contains(60, 60) == false);
        REQUIRE(t.contains(40, 40) == false);
    }

    constexpr auto emptyMultiMapTest = []{
        ConstexprMultiMap<int, int, 64> t;
        return t;
    }();

    static_assert(emptyMultiMapTest.isEmpty());
    static_assert(!emptyMultiMapTest.contains(10));
    static_assert(!emptyMultiMapTest.contains(10, 10));


    TEST_CASE("PutMultipleValues", "[constexpr_multimap]") {
        ConstexprMultiMap<int, int, 64> t;

        t.put(60, 1);
        t.put(60, 2);
        t.put(60, 3);

        REQUIRE(t.getSize() == 1);
        REQUIRE(t.contains(60, 1) == true);
        REQUIRE(t.contains(60, 2) == true);
        REQUIRE(t.contains(60, 3) == true);
        REQUIRE(t.contains(60, 0) == false);
    }

    constexpr auto populatedMultiMapTest = []{
        ConstexprMultiMap<int, int, 64> t;

        t.put(10, 100);
        t.put(10, 101);
        t.put(10, 110);
        t.put(50, 1);

        return t;
    }();

    static_assert(!populatedMultiMapTest.isEmpty());
    static_assert(populatedMultiMapTest.getSize() == 2);
    static_assert(populatedMultiMapTest.contains(10));
    static_assert(!populatedMultiMapTest.get(10).isEmpty());
    static_assert(populatedMultiMapTest.get(10).getSize() == 3);
    static_assert(populatedMultiMapTest.contains(10, 100));
    static_assert(populatedMultiMapTest.contains(10, 101));
    static_assert(populatedMultiMapTest.contains(10, 110));
    static_assert(!populatedMultiMapTest.contains(10, 50));
    static_assert(!populatedMultiMapTest.get(50).isEmpty());
    static_assert(populatedMultiMapTest.get(50).getSize() == 1);
    static_assert(populatedMultiMapTest.contains(50, 1));
    static_assert(!populatedMultiMapTest.contains(50, 2));
}
