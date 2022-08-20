#include <catch2/catch_test_macros.hpp>

#include <container/Vector.hpp>


namespace {
    TEST_CASE("EmptyVector", "[vector]") {
        const Vector<uint32_t,3> vector({});
        REQUIRE(0u == vector.size());
        REQUIRE(3u == vector.getCapacity());
    }

    TEST_CASE("OverCapacity", "[vector]") {
        Vector<uint32_t, 1> vector({0x3, 0xdeadbeef});
        REQUIRE(0u == vector.size());
        REQUIRE(vector.isEmpty());
    }


    TEST_CASE("NonConstVector", "[vector]") {
        Vector<uint32_t,6> vector({0x13,0x8,0x43,0x1024,0xdeadbeef,0x600});

        REQUIRE(6u == vector.size());

        REQUIRE(0x13 == vector[0]);
        REQUIRE(0x8 == vector[1]);
        REQUIRE(0x43 == vector[2]);
        REQUIRE(0x1024 == vector[3]);
        REQUIRE(0xdeadbeef == vector[4]);
        REQUIRE(0x600 == vector[5]);

        vector[2] = 0x55;
        REQUIRE(0x55 == vector[2]);
    }

    TEST_CASE("ConstVector", "[vector]") {
        const Vector<uint32_t,7> vector({0xFF0F, 0x1420, 0x5530});

        REQUIRE(0xFF0F == vector[0]);
        REQUIRE(0x1420 == vector[1]);
        REQUIRE(0x5530 == vector[2]);
    }

    TEST_CASE("OutOfBounds", "[vector]") {
        Vector<uint32_t,15> vector({0x30,0x75,0x44,0xAB,0x88});

        vector[10] = 0u;
    }

    TEST_CASE("OutOfBoundsConst", "[vector]") {
        const Vector<uint32_t,9> vector;

        const uint32_t& x = vector[8];
    }

    TEST_CASE("OutOfBoundsEdge", "[vector]") {
        Vector<uint32_t,11> vector({0x12,0x33,0x78});

        vector[3] = 0u;
    }

    TEST_CASE("OutOfBoundsEmpty", "[vector]") {
        Vector<uint32_t,10> vector;

        vector[0] = 0xdeadbeef;
    }

    TEST_CASE("VectorIterator", "[vector]") {
        Vector<uint32_t,12> vector({0x11,0x22,0x33,0x44,0x55,0x66});

        for (auto &item : vector) {
            item++;
        }

        auto iter = vector.begin();
        REQUIRE(0x12 == *iter); ++iter;
        REQUIRE(iter != vector.end());
        REQUIRE(0x23 == *iter); ++iter;
        REQUIRE(iter != vector.end());
        REQUIRE(0x34 == *iter); ++iter;
        REQUIRE(iter != vector.end());
        REQUIRE(0x45 == *iter); ++iter;
        REQUIRE(iter != vector.end());
        REQUIRE(0x56 == *iter); ++iter;
        REQUIRE(iter != vector.end());
        REQUIRE(0x67 == *iter); ++iter;
        REQUIRE(iter == vector.end());
    }

    TEST_CASE("VectorConstIterator", "[vector]") {
        const Vector<uint32_t,7> vector({0x54, 0x70, 0x31});

        auto iter = vector.begin();

        REQUIRE(0x54 == *iter); ++iter;
        REQUIRE(iter != vector.end());
        REQUIRE(0x70 == *iter); ++iter;
        REQUIRE(iter != vector.end());
        REQUIRE(0x31 == *iter); ++iter;
        REQUIRE(iter == vector.end());
    }

    TEST_CASE("VectorPush", "[vector]") {
        Vector<uint32_t,6> vector({0x1, 0x12, 0x123, 0x1234});

        REQUIRE(4u == vector.size());
        REQUIRE_FALSE(vector.isFull());
        vector.push(0x12345);
        REQUIRE(5u == vector.size());
        REQUIRE(0x12345 == vector[4]);
        REQUIRE_FALSE(vector.isFull());
        vector.push(0x123456);
        REQUIRE(6u == vector.size());
        REQUIRE(0x123456 == vector[5]);
        REQUIRE(vector.isFull());

        vector.push(0xF5);
    }

    TEST_CASE("VectorPop", "[vector]") {
        Vector<uint32_t,14> vector({0xA, 0xB, 0xC, 0xD, 0xE, 0xF});

        REQUIRE(0xF == vector.pop());
        REQUIRE(5u == vector.size());
        REQUIRE_FALSE(vector.isEmpty());
        REQUIRE(0xE == vector.pop());
        REQUIRE(0xD == vector.pop());
        REQUIRE(0xC == vector.pop());
        REQUIRE(0xB == vector.pop());
        REQUIRE_FALSE(vector.isEmpty());
        REQUIRE(0xA == vector.pop());
        REQUIRE(vector.isEmpty());

        std::ignore = vector.pop();
    }



    TEST_CASE("VectorEqualOperator", "[vector]") {
        Vector<uint32_t, 10> a({4, 9, 2, 3, 1});
        Vector<uint32_t, 10> b({4, 9, 2, 3, 1});

        Vector<uint32_t, 10> c({4, 9, 2, 3, 1, 0});
        Vector<uint32_t, 10> d({4, 9, 2, 3});
        Vector<uint32_t, 10> e({1, 9, 2, 3, 1});

        REQUIRE(a == b);
        REQUIRE_FALSE(a == c);
        REQUIRE_FALSE(a == d);
        REQUIRE_FALSE(a == e);
    }

    TEST_CASE("VectorNotEqualOperator", "[vector]") {
        Vector<uint32_t, 10> a({4, 9, 2, 3, 1});
        Vector<uint32_t, 10> b({4, 9, 2, 3, 1});

        Vector<uint32_t, 10> c({4, 9, 2, 3, 1, 0});
        Vector<uint32_t, 10> d({4, 9, 2, 3});
        Vector<uint32_t, 10> e({1, 9, 2, 3, 1});

        REQUIRE_FALSE(a != b);
        REQUIRE(a != c);
        REQUIRE(a != d);
        REQUIRE(a != e);
    }

    TEST_CASE("VectorConcatenate", "[vector]") {
        Vector<uint32_t, 10> lhs({1, 2, 3});
        Vector<uint32_t, 10> rhs({4, 5});

        Vector<uint32_t, 10> cat({1, 2, 3, 4, 5});

        REQUIRE(lhs + rhs == cat);
    }
}

