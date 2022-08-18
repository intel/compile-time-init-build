#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <core/mock/MockLogger.hpp>

#include <container/Array.hpp>

using ::testing::_;


class ArrayTest : public ::testing::Test {
protected:
    MockLogger logger;

    void SetUp() override {
        mockLoggerPtr = &logger;
        EXPECT_CALL(logger, fatal(_)).Times(0);
        EXPECT_CALL(logger, fatal(_,_,_)).Times(0);
    }
};

TEST_F(ArrayTest, EmptyArraySize) {
    const Array<std::uint32_t, 0> array{};
    ASSERT_EQ(0, array.size());
}

TEST_F(ArrayTest, SizeMismatch) {
    EXPECT_CALL(logger, fatal(_, _, _));

    const Array<std::uint32_t, 3> array({8, 9, 10, 12});
}

TEST_F(ArrayTest, ArrayAccess) {
    Array<std::uint32_t, 4> array;

    array[0] = 0xba5eba11u;
    ASSERT_EQ(0xba5eba11u, array[0]);
}

TEST_F(ArrayTest, OutOfBoundsAccess) {
    Array<std::uint32_t, 4> array;

    EXPECT_CALL(logger, fatal(_, _, _));

    array[10] = 0u;
}

TEST_F(ArrayTest, OutOfBoundsConstGet) {
    const Array<std::uint32_t, 4> array;

    EXPECT_CALL(logger, fatal(_, _, _));

    const std::uint32_t& x = array[10];
}

TEST_F(ArrayTest, InBoundsConstGet) {
    const Array<std::uint32_t, 4> array({8, 9, 10, 12});

    ASSERT_EQ(12, array[3]);
}

TEST_F(ArrayTest, OutOfBoundsAccessClose) {
    Array<std::uint32_t, 4> array;

    EXPECT_CALL(logger, fatal(_, _, _));

    array[4] = 0u;
}

TEST_F(ArrayTest, OutOfBoundsAccessEmpty) {
    Array<std::uint32_t, 0> array;

    EXPECT_CALL(logger, fatal(_, _, _));

    array[0] = 0u;
}

TEST_F(ArrayTest, InitializationWithBuiltInArray) {
    Array<std::uint32_t, 4> array({3u, 1u, 4u, 1u});

    ASSERT_EQ(3u, array[0]);
    ASSERT_EQ(1u, array[1]);
    ASSERT_EQ(4u, array[2]);
    ASSERT_EQ(1u, array[3]);
}

TEST_F(ArrayTest, StaticInBounds) {
    const Array<std::uint32_t, 4> array({8u, 3u, 1u, 9u});

    ASSERT_EQ(4u, array.size());
    ASSERT_EQ(8u, array.get<0>());
    ASSERT_EQ(3u, array.get<1>());
    ASSERT_EQ(1u, array.get<2>());
    ASSERT_EQ(9u, array.get<3>());
}

TEST_F(ArrayTest, ForEach) {
    Array<std::uint32_t, 5> array({1u, 1u, 2u, 3u, 5u});

    for (auto item : array) {
        // ensure that foreach can compile
    }

    auto iter = array.begin();
    ASSERT_EQ(1u, *iter); ++iter;
    ASSERT_TRUE(iter != array.end());
    ASSERT_EQ(1u, *iter); ++iter;
    ASSERT_TRUE(iter != array.end());
    ASSERT_EQ(2u, *iter); ++iter;
    ASSERT_TRUE(iter != array.end());
    ASSERT_EQ(3u, *iter); ++iter;
    ASSERT_TRUE(iter != array.end());
    ASSERT_EQ(5u, *iter); ++iter;
    ASSERT_TRUE(iter == array.end());
}
