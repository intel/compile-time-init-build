#include "log.hpp"

#include <container/vector.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <type_traits>

TEST_CASE("EmptyVector", "[vector]") {
    cib::vector<uint32_t, 3> const vector{};
    CHECK(0u == vector.size());
    CHECK(3u == vector.capacity());
}

TEST_CASE("CTAD", "[vector]") {
    cib::vector vector{1, 2, 3, 4, 5, 6};
    static_assert(std::is_same_v<decltype(vector), cib::vector<int, 6>>);
}

TEST_CASE("NonConstVector", "[vector]") {
    cib::vector<uint32_t, 6> vector{0x13, 0x8, 0x43, 0x1024, 0xdeadbeef, 0x600};

    REQUIRE(6u == vector.size());
    CHECK(0x13 == vector[0]);
    CHECK(0x8 == vector[1]);
    CHECK(0x43 == vector[2]);
    CHECK(0x1024 == vector[3]);
    CHECK(0xdeadbeef == vector[4]);
    CHECK(0x600 == vector[5]);

    vector[2] = 0x55;
    CHECK(0x55 == vector[2]);
    CHECK(0x55 == get<2>(vector));
}

TEST_CASE("ConstVector", "[vector]") {
    cib::vector<uint32_t, 7> const vector{0xFF0F, 0x1420, 0x5530};

    REQUIRE(3u == vector.size());
    CHECK(0xFF0F == vector[0]);
    CHECK(0x1420 == vector[1]);
    CHECK(0x5530 == vector[2]);
    CHECK(0x5530 == get<2>(vector));
}

TEST_CASE("OutOfBounds", "[vector]") {
    cib::vector<uint32_t, 15> vector{0x30, 0x75, 0x44, 0xAB, 0x88};
    CHECK_THROWS_AS((vector[10] = 0), test_log_config::exception);
}

TEST_CASE("OutOfBoundsConst", "[vector]") {
    cib::vector<uint32_t, 9> const vector;
    CHECK_THROWS_AS(vector[8], test_log_config::exception);
}

TEST_CASE("OutOfBoundsEdge", "[vector]") {
    cib::vector<uint32_t, 11> vector{0x12, 0x33, 0x78};
    CHECK_THROWS_AS((vector[3] = 0), test_log_config::exception);
}

TEST_CASE("OutOfBoundsEmpty", "[vector]") {
    cib::vector<uint32_t, 10> vector;
    CHECK_THROWS_AS((vector[0] = 0xdeadbeef), test_log_config::exception);
}

TEST_CASE("VectorIterator", "[vector]") {
    cib::vector<uint32_t, 12> vector{0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

    for (auto &item : vector) {
        item++;
    }

    auto iter = vector.begin();
    CHECK(0x12 == *iter);
    ++iter;
    REQUIRE(iter != vector.end());
    CHECK(0x23 == *iter);
    ++iter;
    REQUIRE(iter != vector.end());
    CHECK(0x34 == *iter);
    ++iter;
    REQUIRE(iter != vector.end());
    CHECK(0x45 == *iter);
    ++iter;
    REQUIRE(iter != vector.end());
    CHECK(0x56 == *iter);
    ++iter;
    REQUIRE(iter != vector.end());
    CHECK(0x67 == *iter);
    ++iter;
    CHECK(iter == vector.end());
}

TEST_CASE("VectorConstIterator", "[vector]") {
    cib::vector<uint32_t, 7> const vector{0x54, 0x70, 0x31};

    auto iter = vector.begin();
    CHECK(0x54 == *iter);
    ++iter;
    REQUIRE(iter != vector.end());
    CHECK(0x70 == *iter);
    ++iter;
    REQUIRE(iter != vector.end());
    CHECK(0x31 == *iter);
    ++iter;
    CHECK(iter == vector.end());
}

TEST_CASE("VectorPushBack", "[vector]") {
    cib::vector<uint32_t, 6> vector{0x1, 0x12, 0x123, 0x1234};

    CHECK(4u == vector.size());
    REQUIRE_FALSE(vector.full());
    vector.push_back(0x12345);
    REQUIRE(5u == vector.size());
    CHECK(0x12345 == vector[4]);
    REQUIRE_FALSE(vector.full());
    vector.push_back(0x123456);
    REQUIRE(6u == vector.size());
    CHECK(0x123456 == vector[5]);
    REQUIRE(vector.full());
    CHECK_THROWS_AS(vector.push_back(0xF5), test_log_config::exception);
}

TEST_CASE("VectorPopBack", "[vector]") {
    cib::vector<uint32_t, 14> vector{0xA, 0xB, 0xC, 0xD, 0xE, 0xF};

    CHECK(0xF == vector.pop_back());
    CHECK(5u == vector.size());
    REQUIRE_FALSE(vector.empty());
    CHECK(0xE == vector.pop_back());
    CHECK(0xD == vector.pop_back());
    CHECK(0xC == vector.pop_back());
    CHECK(0xB == vector.pop_back());
    REQUIRE_FALSE(vector.empty());
    CHECK(0xA == vector.pop_back());
    REQUIRE(vector.empty());
    CHECK_THROWS_AS(vector.pop_back(), test_log_config::exception);
}

TEST_CASE("VectorEqualOperator", "[vector]") {
    cib::vector<uint32_t, 10> a{4, 9, 2, 3, 1};
    cib::vector<uint32_t, 10> b{4, 9, 2, 3, 1};

    cib::vector<uint32_t, 10> c{4, 9, 2, 3, 1, 0};
    cib::vector<uint32_t, 10> d{4, 9, 2, 3};
    cib::vector<uint32_t, 10> e{1, 9, 2, 3, 1};

    CHECK(a == b);
    CHECK_FALSE(a == c);
    CHECK_FALSE(a == d);
    CHECK_FALSE(a == e);
}

TEST_CASE("VectorNotEqualOperator", "[vector]") {
    cib::vector<uint32_t, 10> a{4, 9, 2, 3, 1};
    cib::vector<uint32_t, 10> b{4, 9, 2, 3, 1};

    cib::vector<uint32_t, 10> c{4, 9, 2, 3, 1, 0};
    cib::vector<uint32_t, 10> d{4, 9, 2, 3};
    cib::vector<uint32_t, 10> e{1, 9, 2, 3, 1};

    CHECK_FALSE(a != b);
    CHECK(a != c);
    CHECK(a != d);
    CHECK(a != e);
}

TEST_CASE("CapacityZeroVector", "[vector]") {
    cib::vector<int, 0> const vector{};
    CHECK(0u == vector.size());
    CHECK(0u == vector.capacity());
    CHECK(vector.empty());
    CHECK(vector.full());
    CHECK(vector.begin() == vector.end());
    CHECK(vector == cib::vector<int, 0>{});
}

TEST_CASE("Overwrite", "[vector]") {
    cib::vector<int, 5> v{1, 2, 3, 4, 5};
    resize_and_overwrite(v, [](int *dest, std::size_t max_size) {
        CHECK(max_size == 5);
        *dest = 42;
        return 1u;
    });
    REQUIRE(v.size() == 1u);
    CHECK(v[0] == 42);
}
