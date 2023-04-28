#include "log.hpp"

#include <container/constexpr_set.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
TEST_CASE("EmptyAndSize", "[cib::constexpr_set]") {
    cib::constexpr_set<int, 64> t;

    CHECK(t.size() == 0);
    CHECK(t.empty() == true);
}

constexpr auto emptySetTest = [] {
    cib::constexpr_set<int, 64> t;
    return t;
}();

static_assert(emptySetTest.empty());
static_assert(!emptySetTest.contains(10));

TEST_CASE("CTAD", "[cib::constexpr_set]") {
    cib::constexpr_set set{1, 2, 3, 4, 5, 6};
    static_assert(std::is_same_v<decltype(set), cib::constexpr_set<int, 6>>);
}

TEST_CASE("TestContains", "[cib::constexpr_set]") {
    cib::constexpr_set<int, 64> t;

    CHECK(t.contains(10) == false);

    t.add(10);

    CHECK(t.size() == 1);
    CHECK(t.empty() == false);
    CHECK(t.contains(10) == true);
    CHECK(t.contains(11) == false);
}

TEST_CASE("Create", "[cib::constexpr_set]") {
    cib::constexpr_set<int, 64> t{1, 2, 3};

    CHECK(t.size() == 3);
    CHECK(t.empty() == false);
    CHECK(t.contains(1) == true);
    CHECK(t.contains(2) == true);
    CHECK(t.contains(3) == true);
    CHECK(t.contains(11) == false);
}

TEST_CASE("TestMultipleAdd", "[cib::constexpr_set]") {
    cib::constexpr_set<int, 64> t;

    t.add(10);
    t.add(10);

    CHECK(t.size() == 1);
    CHECK(t.empty() == false);
    CHECK(t.contains(10) == true);
}

constexpr auto populatedSetTest = [] {
    cib::constexpr_set<int, 64> t;
    t.add(10);
    t.add(10);
    return t;
}();

static_assert(!populatedSetTest.empty());
static_assert(populatedSetTest.contains(10));
static_assert(!populatedSetTest.contains(11));

TEST_CASE("TestRemoveEverything", "[cib::constexpr_set]") {
    cib::constexpr_set<int, 64> t;

    t.add(10);
    t.remove(10);

    CHECK(t.size() == 0);
    CHECK(t.empty() == true);
    CHECK(t.contains(10) == false);
}

TEST_CASE("TestRemoveSome", "[cib::constexpr_set]") {
    cib::constexpr_set<int, 64> t;

    t.add(10);
    t.add(11);
    t.remove(10);

    CHECK(t.size() == 1);
    CHECK(t.empty() == false);
    CHECK(t.contains(10) == false);
    CHECK(t.contains(11) == true);
}

constexpr auto testSetRemove = [] {
    cib::constexpr_set<int, 64> t;
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

static_assert(!testSetRemove.empty());
static_assert(testSetRemove.contains(12));
static_assert(testSetRemove.contains(40));
static_assert(testSetRemove.contains(42));

static_assert(!testSetRemove.contains(10));
static_assert(!testSetRemove.contains(11));
static_assert(!testSetRemove.contains(32));
static_assert(!testSetRemove.contains(56));

TEST_CASE("TestRemoveAll", "[cib::constexpr_set]") {
    cib::constexpr_set<int, 64> t;
    t.add(11);
    t.add(12);
    t.add(40);
    t.add(42);
    t.add(56);

    cib::constexpr_set<int, 64> setToRemove;
    setToRemove.add(10);
    setToRemove.add(11);
    setToRemove.add(32);
    setToRemove.add(56);

    t.remove_all(setToRemove);

    CHECK(t.size() == 3);
    CHECK(t.empty() == false);
    CHECK(t.contains(12) == true);
    CHECK(t.contains(40) == true);
    CHECK(t.contains(42) == true);
    CHECK(t.contains(10) == false);
    CHECK(t.contains(11) == false);
    CHECK(t.contains(32) == false);
    CHECK(t.contains(56) == false);
}

constexpr auto testSetRemoveAll = [] {
    cib::constexpr_set<int, 64> t;
    t.add(10);
    t.add(11);
    t.add(12);
    t.add(32);
    t.add(40);
    t.add(42);
    t.add(56);

    cib::constexpr_set<int, 64> setToRemove;
    setToRemove.add(10);
    setToRemove.add(11);
    setToRemove.add(32);
    setToRemove.add(56);
    t.remove_all(setToRemove);

    return t;
}();

static_assert(!testSetRemoveAll.empty());
static_assert(testSetRemoveAll.contains(12));
static_assert(testSetRemoveAll.contains(40));
static_assert(testSetRemoveAll.contains(42));
static_assert(!testSetRemoveAll.contains(10));
static_assert(!testSetRemoveAll.contains(11));
static_assert(!testSetRemoveAll.contains(32));
static_assert(!testSetRemoveAll.contains(56));
} // namespace
