#include "gtest/gtest.h"

#include <container/ConstexprMap.hpp>

TEST(ConstexprMapTest, EmptyAndSize) {
    ConstexprMap<int, int, 64> t;

    EXPECT_EQ(t.getSize(), 0);
    EXPECT_EQ(t.isEmpty(), true);

    t.put(10, 50);

    EXPECT_EQ(t.getSize(), 1);
    EXPECT_EQ(t.isEmpty(), false);
}

TEST(ConstexprMapTest, ContainsAndGet) {
    ConstexprMap<int, int, 64> t;
    t.put(10, 50);
    t.put(11, 100);

    EXPECT_EQ(t.contains(10), true);
    EXPECT_EQ(t.get(10), 50);
    EXPECT_EQ(t.contains(11), true);
    EXPECT_EQ(t.get(11), 100);
    EXPECT_EQ(t.contains(12), false);
}

TEST(ConstexprMapTest, ConstGet) {
    const ConstexprMap<int, int, 64> t = []{
        ConstexprMap<int, int, 64> t;
        t.put(10, 50);
        t.put(11, 100);
        return t;
    }();

    EXPECT_EQ(t.contains(10), true);
    EXPECT_EQ(t.get(10), 50);
    EXPECT_EQ(t.contains(11), true);
    EXPECT_EQ(t.get(11), 100);
    EXPECT_EQ(t.contains(12), false);
}

TEST(ConstexprMapTest, UpdateExistingKey) {
    ConstexprMap<int, int, 64> t;
    t.put(13, 500);
    t.put(13, 700);

    EXPECT_EQ(t.contains(13), true);
    EXPECT_EQ(t.get(13), 700);
}

TEST(ConstexprMapTest, Pop) {
    ConstexprMap<int, int, 64> t;
    t.put(13, 500);

    auto entry = t.pop();

    EXPECT_EQ(t.getSize(), 0);
    EXPECT_EQ(t.isEmpty(), true);
    EXPECT_EQ(entry.key, 13);
    EXPECT_EQ(entry.value, 500);
}

TEST(ConstexprMapTest, Remove) {
    ConstexprMap<int, int, 64> t;
    t.put(13, 500);
    t.put(18, 600);
    t.put(19, 700);

    t.remove(18);

    EXPECT_EQ(t.getSize(), 2);
    EXPECT_EQ(t.contains(18), false);
    EXPECT_EQ(t.get(13), 500);
    EXPECT_EQ(t.get(19), 700);
}

TEST(ConstexprMapTest, RemoveNonExistantKey) {
    ConstexprMap<int, int, 64> t;
    t.put(13, 500);
    t.put(18, 600);

    t.remove(50);

    EXPECT_EQ(t.getSize(), 2);
    EXPECT_EQ(t.contains(50), false);
    EXPECT_EQ(t.get(13), 500);
    EXPECT_EQ(t.get(18), 600);
}

TEST(ConstexprMapTest, EmptyIterators) {
    ConstexprMap<int, int, 64> t;

    EXPECT_EQ(t.begin(), t.end());
}

TEST(ConstexprMapTest, NonEmptyIterators) {
    ConstexprMap<int, int, 64> t;

    t.put(18, 600);

    EXPECT_EQ(t.begin() + 1, t.end());
}


constexpr auto emptyMapTest = []{
    ConstexprMap<int, int, 64> t;
    return t;
}();

static_assert(emptyMapTest.isEmpty());
static_assert(!emptyMapTest.contains(10));


constexpr auto populatedMapTest = []{
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


constexpr auto testMapRemove = []{
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


constexpr auto testSetUpdateValue = []{
    ConstexprMap<int, int, 64> t;
    t.put(10, 50);
    t.put(10, 100);
    return t;
}();

static_assert(!testSetUpdateValue.isEmpty());
static_assert(testSetUpdateValue.contains(10));
static_assert(testSetUpdateValue.get(10) == 100);


constexpr auto testMapRemoveFirst = []{
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
