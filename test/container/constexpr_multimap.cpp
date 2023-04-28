#include <container/constexpr_multimap.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
TEST_CASE("EmptyAndSize", "[cib::constexpr_multimap]") {
    cib::constexpr_multimap<int, int, 64> t;

    CHECK(t.size() == 0);
    CHECK(t.empty() == true);
}

TEST_CASE("PutAndContains", "[cib::constexpr_multimap]") {
    cib::constexpr_multimap<int, int, 64> t;

    t.put(60, 40);

    CHECK(t.size() == 1);
    CHECK(t.empty() == false);
    CHECK(t.contains(60) == true);
    CHECK(t.contains(40) == false);
    CHECK(t.contains(60, 40) == true);
    CHECK(t.contains(60, 60) == false);
    CHECK(t.contains(40, 40) == false);
}

constexpr auto emptyMultiMapTest = [] {
    cib::constexpr_multimap<int, int, 64> t;
    return t;
}();

static_assert(emptyMultiMapTest.empty());
static_assert(!emptyMultiMapTest.contains(10));
static_assert(!emptyMultiMapTest.contains(10, 10));

TEST_CASE("PutMultipleValues", "[cib::constexpr_multimap]") {
    cib::constexpr_multimap<int, int, 64> t;

    t.put(60, 1);
    t.put(60, 2);
    t.put(60, 3);

    CHECK(t.size() == 1);
    CHECK(t.contains(60, 1) == true);
    CHECK(t.contains(60, 2) == true);
    CHECK(t.contains(60, 3) == true);
    CHECK(t.contains(60, 0) == false);
}

constexpr auto populatedMultiMapTest = [] {
    cib::constexpr_multimap<int, int, 64> t;

    t.put(10, 100);
    t.put(10, 101);
    t.put(10, 110);
    t.put(50, 1);

    return t;
}();

static_assert(!populatedMultiMapTest.empty());
static_assert(populatedMultiMapTest.size() == 2);
static_assert(populatedMultiMapTest.contains(10));
static_assert(!populatedMultiMapTest.get(10).empty());
static_assert(populatedMultiMapTest.get(10).size() == 3);
static_assert(populatedMultiMapTest.contains(10, 100));
static_assert(populatedMultiMapTest.contains(10, 101));
static_assert(populatedMultiMapTest.contains(10, 110));
static_assert(!populatedMultiMapTest.contains(10, 50));
static_assert(!populatedMultiMapTest.get(50).empty());
static_assert(populatedMultiMapTest.get(50).size() == 1);
static_assert(populatedMultiMapTest.contains(50, 1));
static_assert(!populatedMultiMapTest.contains(50, 2));
} // namespace
