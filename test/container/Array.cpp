#include <container/Array.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
TEST_CASE("EmptyArraySize", "[array]") {
    Array<std::uint32_t, 0> const array{};
    REQUIRE(0 == array.size());
}

TEST_CASE("SizeMismatch", "[array]") {
    Array<std::uint32_t, 3> const array({8, 9, 10, 12});
}

TEST_CASE("ArrayAccess", "[array]") {
    Array<std::uint32_t, 4> array;

    array[0] = 0xba5eba11u;
    REQUIRE(0xba5eba11u == array[0]);
}

TEST_CASE("OutOfBoundsAccess", "[array]") {
    Array<std::uint32_t, 4> array;

    array[10] = 0u;
}

TEST_CASE("OutOfBoundsConstGet", "[array]") {
    Array<std::uint32_t, 4> const array;

    std::uint32_t const &x = array[10];
}

TEST_CASE("InBoundsConstGet", "[array]") {
    Array<std::uint32_t, 4> const array({8, 9, 10, 12});

    REQUIRE(12 == array[3]);
}

TEST_CASE("OutOfBoundsAccessClose", "[array]") {
    Array<std::uint32_t, 4> array;

    array[4] = 0u;
}

TEST_CASE("OutOfBoundsAccessEmpty", "[array]") {
    Array<std::uint32_t, 0> array;

    array[0] = 0u;
}

TEST_CASE("InitializationWithBuiltInArray", "[array]") {
    Array<std::uint32_t, 4> array({3u, 1u, 4u, 1u});

    REQUIRE(3u == array[0]);
    REQUIRE(1u == array[1]);
    REQUIRE(4u == array[2]);
    REQUIRE(1u == array[3]);
}

TEST_CASE("StaticInBounds", "[array]") {
    Array<std::uint32_t, 4> const array({8u, 3u, 1u, 9u});

    REQUIRE(4u == array.size());
    REQUIRE(8u == array.get<0>());
    REQUIRE(3u == array.get<1>());
    REQUIRE(1u == array.get<2>());
    REQUIRE(9u == array.get<3>());
}

TEST_CASE("ForEach", "[array]") {
    Array<std::uint32_t, 5> array({1u, 1u, 2u, 3u, 5u});

    for (auto item : array) {
        // ensure that foreach can compile
    }

    auto iter = array.begin();
    REQUIRE(1u == *iter);
    ++iter;
    REQUIRE(iter != array.end());
    REQUIRE(1u == *iter);
    ++iter;
    REQUIRE(iter != array.end());
    REQUIRE(2u == *iter);
    ++iter;
    REQUIRE(iter != array.end());
    REQUIRE(3u == *iter);
    ++iter;
    REQUIRE(iter != array.end());
    REQUIRE(5u == *iter);
    ++iter;
    REQUIRE(iter == array.end());
}
} // namespace