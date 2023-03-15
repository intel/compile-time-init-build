#include "log.hpp"

#include <container/Vector.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
TEST_CASE("EmptyVector", "[vector]") {
    const Vector<uint32_t, 3> vector({});
    CHECK(0u == vector.size());
    CHECK(3u == vector.getCapacity());
}

TEST_CASE("CTAD", "[vector]") {
    Vector vector{1, 2, 3, 4, 5, 6};
    static_assert(std::is_same_v<decltype(vector), Vector<int, 6>>);
}

TEST_CASE("CreateOverCapacity", "[vector]") {
    constexpr auto make_vector = []() {
        Vector<uint32_t, 1> const vector{0x3, 0xdeadbeef};
        return vector;
    };
    CHECK_THROWS_AS(make_vector(), test_log_config::exception);

    test_log_config::prevent_throws nothrows{};
    auto const vector = make_vector();
    CHECK(0u == vector.size());
    CHECK(vector.isEmpty());
}

TEST_CASE("NonConstVector", "[vector]") {
    Vector<uint32_t, 6> vector{0x13, 0x8, 0x43, 0x1024, 0xdeadbeef, 0x600};

    REQUIRE(6u == vector.size());
    CHECK(0x13 == vector[0]);
    CHECK(0x8 == vector[1]);
    CHECK(0x43 == vector[2]);
    CHECK(0x1024 == vector[3]);
    CHECK(0xdeadbeef == vector[4]);
    CHECK(0x600 == vector[5]);

    vector[2] = 0x55;
    CHECK(0x55 == vector[2]);
}

TEST_CASE("ConstVector", "[vector]") {
    const Vector<uint32_t, 7> vector{0xFF0F, 0x1420, 0x5530};

    REQUIRE(3u == vector.size());
    CHECK(0xFF0F == vector[0]);
    CHECK(0x1420 == vector[1]);
    CHECK(0x5530 == vector[2]);
}

TEST_CASE("OutOfBounds", "[vector]") {
    Vector<uint32_t, 15> vector{0x30, 0x75, 0x44, 0xAB, 0x88};
    CHECK_THROWS_AS((vector[10] = 0), test_log_config::exception);
}

TEST_CASE("OutOfBoundsConst", "[vector]") {
    const Vector<uint32_t, 9> vector;
    CHECK_THROWS_AS(vector[8], test_log_config::exception);
}

TEST_CASE("OutOfBoundsEdge", "[vector]") {
    Vector<uint32_t, 11> vector{0x12, 0x33, 0x78};
    CHECK_THROWS_AS((vector[3] = 0), test_log_config::exception);
}

TEST_CASE("OutOfBoundsEmpty", "[vector]") {
    Vector<uint32_t, 10> vector;
    CHECK_THROWS_AS((vector[0] = 0xdeadbeef), test_log_config::exception);
}

TEST_CASE("VectorIterator", "[vector]") {
    Vector<uint32_t, 12> vector{0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

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
    const Vector<uint32_t, 7> vector{0x54, 0x70, 0x31};

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

TEST_CASE("VectorPush", "[vector]") {
    Vector<uint32_t, 6> vector{0x1, 0x12, 0x123, 0x1234};

    CHECK(4u == vector.size());
    REQUIRE_FALSE(vector.isFull());
    vector.push(0x12345);
    REQUIRE(5u == vector.size());
    CHECK(0x12345 == vector[4]);
    REQUIRE_FALSE(vector.isFull());
    vector.push(0x123456);
    REQUIRE(6u == vector.size());
    CHECK(0x123456 == vector[5]);
    REQUIRE(vector.isFull());
    CHECK_THROWS_AS(vector.push(0xF5), test_log_config::exception);
}

TEST_CASE("VectorPop", "[vector]") {
    Vector<uint32_t, 14> vector{0xA, 0xB, 0xC, 0xD, 0xE, 0xF};

    CHECK(0xF == vector.pop());
    CHECK(5u == vector.size());
    REQUIRE_FALSE(vector.isEmpty());
    CHECK(0xE == vector.pop());
    CHECK(0xD == vector.pop());
    CHECK(0xC == vector.pop());
    CHECK(0xB == vector.pop());
    REQUIRE_FALSE(vector.isEmpty());
    CHECK(0xA == vector.pop());
    REQUIRE(vector.isEmpty());
    CHECK_THROWS_AS(vector.pop(), test_log_config::exception);
}

TEST_CASE("VectorEqualOperator", "[vector]") {
    Vector<uint32_t, 10> a{4, 9, 2, 3, 1};
    Vector<uint32_t, 10> b{4, 9, 2, 3, 1};

    Vector<uint32_t, 10> c{4, 9, 2, 3, 1, 0};
    Vector<uint32_t, 10> d{4, 9, 2, 3};
    Vector<uint32_t, 10> e{1, 9, 2, 3, 1};

    CHECK(a == b);
    CHECK_FALSE(a == c);
    CHECK_FALSE(a == d);
    CHECK_FALSE(a == e);
}

TEST_CASE("VectorNotEqualOperator", "[vector]") {
    Vector<uint32_t, 10> a{4, 9, 2, 3, 1};
    Vector<uint32_t, 10> b{4, 9, 2, 3, 1};

    Vector<uint32_t, 10> c{4, 9, 2, 3, 1, 0};
    Vector<uint32_t, 10> d{4, 9, 2, 3};
    Vector<uint32_t, 10> e{1, 9, 2, 3, 1};

    CHECK_FALSE(a != b);
    CHECK(a != c);
    CHECK(a != d);
    CHECK(a != e);
}

TEST_CASE("VectorConcatenate", "[vector]") {
    Vector<uint32_t, 10> lhs{1, 2, 3};
    Vector<uint32_t, 10> rhs{4, 5};

    Vector<uint32_t, 10> cat{1, 2, 3, 4, 5};

    CHECK(lhs + rhs == cat);
}
} // namespace
