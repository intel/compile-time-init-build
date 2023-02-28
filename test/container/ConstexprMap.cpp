#include "log.hpp"

#include <container/ConstexprMap.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
TEST_CASE("EmptyAndSize", "[constexpr_map]") {
    ConstexprMap<int, int, 64> t;

    CHECK(t.getSize() == 0);
    CHECK(t.isEmpty() == true);

    t.put(10, 50);

    CHECK(t.getSize() == 1);
    CHECK(t.isEmpty() == false);
}

TEST_CASE("ContainsAndGet", "[constexpr_map]") {
    ConstexprMap<int, int, 64> t;
    t.put(10, 50);
    t.put(11, 100);

    REQUIRE(t.contains(10) == true);
    CHECK(t.get(10) == 50);
    REQUIRE(t.contains(11) == true);
    CHECK(t.get(11) == 100);
    CHECK(t.contains(12) == false);
    CHECK_THROWS_AS(t.get(12), test_log_config::exception);
}

TEST_CASE("ConstGet", "[constexpr_map]") {
    const ConstexprMap<int, int, 64> t = [] {
        ConstexprMap<int, int, 64> m;
        m.put(10, 50);
        m.put(11, 100);
        return m;
    }();

    REQUIRE(t.contains(10) == true);
    CHECK(t.get(10) == 50);
    REQUIRE(t.contains(11) == true);
    CHECK(t.get(11) == 100);
    CHECK(t.contains(12) == false);
    CHECK_THROWS_AS(t.get(12), test_log_config::exception);
}

TEST_CASE("UpdateExistingKey", "[constexpr_map]") {
    ConstexprMap<int, int, 64> t;
    t.put(13, 500);
    t.put(13, 700);

    REQUIRE(t.contains(13) == true);
    CHECK(t.get(13) == 700);
}

TEST_CASE("PutWhenFull", "[constexpr_map]") {
    ConstexprMap<int, int, 1> t;
    t.put(13, 500);
    CHECK_THROWS_AS(t.put(12, 700), test_log_config::exception);
}

TEST_CASE("Pop", "[constexpr_map]") {
    ConstexprMap<int, int, 64> t;
    t.put(13, 500);

    auto entry = t.pop();

    CHECK(t.getSize() == 0);
    CHECK(t.isEmpty() == true);
    CHECK(entry.key == 13);
    CHECK(entry.value == 500);
    CHECK_THROWS_AS(t.pop(), test_log_config::exception);
}

TEST_CASE("Remove", "[constexpr_map]") {
    ConstexprMap<int, int, 64> t;
    t.put(13, 500);
    t.put(18, 600);
    t.put(19, 700);

    t.remove(18);

    CHECK(t.getSize() == 2);
    CHECK(t.contains(18) == false);
    CHECK(t.get(13) == 500);
    CHECK(t.get(19) == 700);
}

TEST_CASE("RemoveNonExistantKey", "[constexpr_map]") {
    ConstexprMap<int, int, 64> t;
    t.put(13, 500);
    t.put(18, 600);

    t.remove(50);

    CHECK(t.getSize() == 2);
    CHECK(t.contains(50) == false);
    CHECK(t.get(13) == 500);
    CHECK(t.get(18) == 600);
}

TEST_CASE("EmptyIterators", "[constexpr_map]") {
    ConstexprMap<int, int, 64> t;

    CHECK(t.begin() == t.end());
}

TEST_CASE("NonEmptyIterators", "[constexpr_map]") {
    ConstexprMap<int, int, 64> t;

    t.put(18, 600);

    CHECK((t.begin() + 1) == t.end());
}

constexpr auto emptyMapTest = [] {
    ConstexprMap<int, int, 64> t;
    return t;
}();

static_assert(emptyMapTest.isEmpty());
static_assert(!emptyMapTest.contains(10));

constexpr auto populatedMapTest = [] {
    ConstexprMap<int, int, 64> t;
    t.put(10, 50);
    t.put(11, 100);
    return t;
}();

static_assert(!populatedMapTest.isEmpty());
static_assert(populatedMapTest.contains(10));
static_assert(populatedMapTest.get(10) == 50);
static_assert(populatedMapTest.contains(11));
static_assert(populatedMapTest.get(11) == 100);
static_assert(!populatedMapTest.contains(12));

constexpr auto testMapRemove = [] {
    ConstexprMap<int, int, 64> t;
    t.put(10, 50);
    t.put(11, 100);
    t.remove(11);
    t.remove(12);
    return t;
}();

static_assert(!testMapRemove.isEmpty());
static_assert(testMapRemove.contains(10));
static_assert(testMapRemove.get(10) == 50);
static_assert(!testMapRemove.contains(11));

constexpr auto testSetUpdateValue = [] {
    ConstexprMap<int, int, 64> t;
    t.put(10, 50);
    t.put(10, 100);
    return t;
}();

static_assert(!testSetUpdateValue.isEmpty());
static_assert(testSetUpdateValue.contains(10));
static_assert(testSetUpdateValue.get(10) == 100);

constexpr auto testMapRemoveFirst = [] {
    ConstexprMap<int, int, 64> t;
    t.put(10, 50);
    t.put(11, 100);
    t.remove(10);
    return t;
}();

static_assert(!testMapRemoveFirst.isEmpty());
static_assert(testMapRemoveFirst.contains(11));
static_assert(testMapRemoveFirst.get(11) == 100);
static_assert(!testMapRemoveFirst.contains(10));
} // namespace
