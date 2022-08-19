#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <core/mock/MockLogger.hpp>
#include <container/Vector.hpp>


using ::testing::_;


class VectorTest : public ::testing::Test {
protected:
    MockLogger logger;

    void SetUp() override {
        mockLoggerPtr = &logger;
        EXPECT_CALL(logger, fatal(_)).Times(0);
        EXPECT_CALL(logger, fatal(_,_,_)).Times(0);
    }
};

TEST_F(VectorTest, EmptyVector) {
    const Vector<uint32_t,3> vector({});
    ASSERT_EQ(0u, vector.size());
    ASSERT_EQ(3u, vector.getCapacity());
}

TEST_F(VectorTest, OverCapacity) {
    EXPECT_CALL(logger, fatal(_, _, _));
    Vector<uint32_t, 1> vector({0x3, 0xdeadbeef});
    ASSERT_EQ(0u, vector.size());
    ASSERT_TRUE(vector.isEmpty());
}


TEST_F(VectorTest, NonConstVector) {
    Vector<uint32_t,6> vector({0x13,0x8,0x43,0x1024,0xdeadbeef,0x600});

    ASSERT_EQ(6u, vector.size());

    ASSERT_EQ(0x13, vector[0]);
    ASSERT_EQ(0x8, vector[1]);
    ASSERT_EQ(0x43, vector[2]);
    ASSERT_EQ(0x1024, vector[3]);
    ASSERT_EQ(0xdeadbeef, vector[4]);
    ASSERT_EQ(0x600, vector[5]);

    vector[2] = 0x55;
    ASSERT_EQ(0x55, vector[2]);
}

TEST_F(VectorTest, ConstVector) {
    const Vector<uint32_t,7> vector({0xFF0F, 0x1420, 0x5530});

    ASSERT_EQ(0xFF0F, vector[0]);
    ASSERT_EQ(0x1420, vector[1]);
    ASSERT_EQ(0x5530, vector[2]);
}

TEST_F(VectorTest, OutOfBounds) {
    Vector<uint32_t,15> vector({0x30,0x75,0x44,0xAB,0x88});

    EXPECT_CALL(logger, fatal(_, _, _));
    vector[10] = 0u;
}

TEST_F(VectorTest, OutOfBoundsConst) {
    const Vector<uint32_t,9> vector;

    EXPECT_CALL(logger, fatal(_, _, _));
    const uint32_t& x = vector[8];
}

TEST_F(VectorTest, OutOfBoundsEdge) {
    Vector<uint32_t,11> vector({0x12,0x33,0x78});

    EXPECT_CALL(logger, fatal(_, _, _));
    vector[3] = 0u;
}

TEST_F(VectorTest, OutOfBoundsEmpty) {
    Vector<uint32_t,10> vector;

    EXPECT_CALL(logger, fatal(_, _, _));
    vector[0] = 0xdeadbeef;
}

TEST_F(VectorTest, VectorIterator) {
    Vector<uint32_t,12> vector({0x11,0x22,0x33,0x44,0x55,0x66});

    for (auto &item : vector) {
        item++;
    }

    auto iter = vector.begin();
    ASSERT_EQ(0x12, *iter); ++iter;
    ASSERT_TRUE(iter != vector.end());
    ASSERT_EQ(0x23, *iter); ++iter;
    ASSERT_TRUE(iter != vector.end());
    ASSERT_EQ(0x34, *iter); ++iter;
    ASSERT_TRUE(iter != vector.end());
    ASSERT_EQ(0x45, *iter); ++iter;
    ASSERT_TRUE(iter != vector.end());
    ASSERT_EQ(0x56, *iter); ++iter;
    ASSERT_TRUE(iter != vector.end());
    ASSERT_EQ(0x67, *iter); ++iter;
    ASSERT_TRUE(iter == vector.end());
}

TEST_F(VectorTest, VectorConstIterator) {
    const Vector<uint32_t,7> vector({0x54, 0x70, 0x31});

    auto iter = vector.begin();

    ASSERT_EQ(0x54, *iter); ++iter;
    ASSERT_TRUE(iter != vector.end());
    ASSERT_EQ(0x70, *iter); ++iter;
    ASSERT_TRUE(iter != vector.end());
    ASSERT_EQ(0x31, *iter); ++iter;
    ASSERT_TRUE(iter == vector.end());
}

TEST_F(VectorTest, VectorPush) {
    Vector<uint32_t,6> vector({0x1, 0x12, 0x123, 0x1234});

    ASSERT_EQ(4u, vector.size());
    ASSERT_FALSE(vector.isFull());
    vector.push(0x12345);
    ASSERT_EQ(5u, vector.size());
    ASSERT_EQ(0x12345, vector[4]);
    ASSERT_FALSE(vector.isFull());
    vector.push(0x123456);
    ASSERT_EQ(6u, vector.size());
    ASSERT_EQ(0x123456, vector[5]);
    ASSERT_TRUE(vector.isFull());

    EXPECT_CALL(logger, fatal(_));
    vector.push(0xF5);
}

TEST_F(VectorTest, VectorPop) {
    Vector<uint32_t,14> vector({0xA, 0xB, 0xC, 0xD, 0xE, 0xF});

    ASSERT_EQ(0xF, vector.pop());
    ASSERT_EQ(5u, vector.size());
    ASSERT_FALSE(vector.isEmpty());
    ASSERT_EQ(0xE, vector.pop());
    ASSERT_EQ(0xD, vector.pop());
    ASSERT_EQ(0xC, vector.pop());
    ASSERT_EQ(0xB, vector.pop());
    ASSERT_FALSE(vector.isEmpty());
    ASSERT_EQ(0xA, vector.pop());
    ASSERT_TRUE(vector.isEmpty());

    EXPECT_CALL(logger, fatal(_));
    std::ignore = vector.pop();
}



TEST_F(VectorTest, VectorEqualOperator) {
    Vector<uint32_t, 10> a({4, 9, 2, 3, 1});
    Vector<uint32_t, 10> b({4, 9, 2, 3, 1});

    Vector<uint32_t, 10> c({4, 9, 2, 3, 1, 0});
    Vector<uint32_t, 10> d({4, 9, 2, 3});
    Vector<uint32_t, 10> e({1, 9, 2, 3, 1});

    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a == c);
    ASSERT_FALSE(a == d);
    ASSERT_FALSE(a == e);
}

TEST_F(VectorTest, VectorNotEqualOperator) {
    Vector<uint32_t, 10> a({4, 9, 2, 3, 1});
    Vector<uint32_t, 10> b({4, 9, 2, 3, 1});

    Vector<uint32_t, 10> c({4, 9, 2, 3, 1, 0});
    Vector<uint32_t, 10> d({4, 9, 2, 3});
    Vector<uint32_t, 10> e({1, 9, 2, 3, 1});

    ASSERT_FALSE(a != b);
    ASSERT_TRUE(a != c);
    ASSERT_TRUE(a != d);
    ASSERT_TRUE(a != e);
}

TEST_F(VectorTest, VectorConcatenate) {
    Vector<uint32_t, 10> lhs({1, 2, 3});
    Vector<uint32_t, 10> rhs({4, 5});

    Vector<uint32_t, 10> cat({1, 2, 3, 4, 5});

    ASSERT_EQ(lhs + rhs, cat);
}

