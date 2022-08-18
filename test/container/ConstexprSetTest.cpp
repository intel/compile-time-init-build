#include "gtest/gtest.h"

#include <container/ConstexprSet.hpp>


TEST(ConstexprSetTest, EmptyAndSize) {
    ConstexprSet<int, 64> t;

    EXPECT_EQ(t.getSize(), 0);
    EXPECT_EQ(t.isEmpty(), true);
}

constexpr auto emptySetTest = []{
    ConstexprSet<int, 64> t;
    return t;
}();

static_assert(emptySetTest.isEmpty());
static_assert(!emptySetTest.contains(10));

TEST(ConstexprSetTest, TestContains) {
    ConstexprSet<int, 64> t;

    EXPECT_EQ(t.contains(10), false);

    t.add(10);

    EXPECT_EQ(t.getSize(), 1);
    EXPECT_EQ(t.isEmpty(), false);
    EXPECT_EQ(t.contains(10), true);
    EXPECT_EQ(t.contains(11), false);
}

TEST(ConstexprSetTest, TestMultipleAdd) {
    ConstexprSet<int, 64> t;

    t.add(10);
    t.add(10);

    EXPECT_EQ(t.getSize(), 1);
    EXPECT_EQ(t.isEmpty(), false);
    EXPECT_EQ(t.contains(10), true);
}

constexpr auto populatedSetTest = []{
    ConstexprSet<int, 64> t;
    t.add(10);
    t.add(10);
    return t;
}();

static_assert(!populatedSetTest.isEmpty());
static_assert(populatedSetTest.contains(10));
static_assert(!populatedSetTest.contains(11));


TEST(ConstexprSetTest, TestRemoveEverything) {
    ConstexprSet<int, 64> t;

    t.add(10);
    t.remove(10);

    EXPECT_EQ(t.getSize(), 0);
    EXPECT_EQ(t.isEmpty(), true);
    EXPECT_EQ(t.contains(10), false);
}

TEST(ConstexprSetTest, TestRemoveSome) {
    ConstexprSet<int, 64> t;

    t.add(10);
    t.add(11);
    t.remove(10);

    EXPECT_EQ(t.getSize(), 1);
    EXPECT_EQ(t.isEmpty(), false);
    EXPECT_EQ(t.contains(10), false);
    EXPECT_EQ(t.contains(11), true);
}

constexpr auto testSetRemove = []{
    ConstexprSet<int, 64> t;
    t.add(10);
    t.add(11);
    t.add(12);
    t.add(32);
    t.add(40);
    t.add(42);
    t.add(56);

    t.remove(10);
    t.remove(11);
    t.remove(11);
    t.remove(32);
    t.remove(56);
    t.remove(56);

    return t;
}();

static_assert(!testSetRemove.isEmpty());
static_assert(testSetRemove.contains(12));
static_assert(testSetRemove.contains(40));
static_assert(testSetRemove.contains(42));

static_assert(!testSetRemove.contains(10));
static_assert(!testSetRemove.contains(11));
static_assert(!testSetRemove.contains(32));
static_assert(!testSetRemove.contains(56));


TEST(ConstexprSetTest, TestRemoveAll) {
    ConstexprSet<int, 64> t;
    t.add(11);
    t.add(12);
    t.add(40);
    t.add(42);
    t.add(56);

    ConstexprSet<int, 64> setToRemove;
    setToRemove.add(10);
    setToRemove.add(11);
    setToRemove.add(32);
    setToRemove.add(56);

    t.removeAll(setToRemove);


    EXPECT_EQ(t.getSize(), 3);
    EXPECT_EQ(t.isEmpty(), false);
    EXPECT_EQ(t.contains(12), true);
    EXPECT_EQ(t.contains(40), true);
    EXPECT_EQ(t.contains(42), true);
    EXPECT_EQ(t.contains(10), false);
    EXPECT_EQ(t.contains(11), false);
    EXPECT_EQ(t.contains(32), false);
    EXPECT_EQ(t.contains(56), false);
}

constexpr auto testSetRemoveAll = []{
    ConstexprSet<int, 64> t;
    t.add(10);
    t.add(11);
    t.add(12);
    t.add(32);
    t.add(40);
    t.add(42);
    t.add(56);

    ConstexprSet<int, 64> setToRemove;
    setToRemove.add(10);
    setToRemove.add(11);
    setToRemove.add(32);
    setToRemove.add(56);
    t.removeAll(setToRemove);

    return t;
}();


static_assert(!testSetRemoveAll.isEmpty());
static_assert(testSetRemoveAll.contains(12));
static_assert(testSetRemoveAll.contains(40));
static_assert(testSetRemoveAll.contains(42));
static_assert(!testSetRemoveAll.contains(10));
static_assert(!testSetRemoveAll.contains(11));
static_assert(!testSetRemoveAll.contains(32));
static_assert(!testSetRemoveAll.contains(56));
