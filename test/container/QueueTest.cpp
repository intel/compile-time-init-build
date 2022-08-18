#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <core/mock/MockLogger.hpp>
#include <HardwareAbstractionLayer.hpp>

#include <container/Queue.hpp>

using ::testing::_;
using ::testing::Return;
using ::testing::Sequence;

namespace {
    class QueueTest : public ::testing::Test {
    protected:
        MockLogger logger;
        MockCriticalSection criticalSection;

        void SetUp() override {
            mockLoggerPtr = &logger;
            mockCriticalSectionPtr = &criticalSection;
            EXPECT_CALL(logger, fatal(_)).Times(0);
            EXPECT_CALL(logger, fatal(_, _)).Times(0);
            EXPECT_CALL(logger, fatal(_, _, _)).Times(0);
        }
    };


    TEST_F(QueueTest, EmptyQueueSize) {
        const Queue<std::uint32_t, 4> q{};
        ASSERT_EQ(0, q.getSize());
        ASSERT_TRUE(q.isEmpty());
        ASSERT_FALSE(q.isFull());
    }

    TEST_F(QueueTest, PutOneSize) {
        Queue<std::uint32_t, 4> q{};

        q.put(10);

        ASSERT_EQ(1, q.getSize());
        ASSERT_FALSE(q.isEmpty());
        ASSERT_FALSE(q.isFull());
    }

    TEST_F(QueueTest, PutOneGetOne) {
        Queue<std::uint32_t, 4> q{};

        q.put(0xc01bead5);

        ASSERT_EQ(0xc01bead5, q.get());
        ASSERT_EQ(0, q.getSize());
        ASSERT_TRUE(q.isEmpty());
        ASSERT_FALSE(q.isFull());
    }

    TEST_F(QueueTest, Full) {
        Queue<std::uint32_t, 4> q{};

        q.put(0xc01bead5);
        q.put(0xcab0b5);
        q.put(0xbadb095);
        q.put(0x5ea54e11);

        ASSERT_EQ(4, q.getSize());
        ASSERT_TRUE(q.isFull());
        ASSERT_FALSE(q.isEmpty());

        ASSERT_EQ(0xc01bead5, q.get());
        ASSERT_EQ(0xcab0b5, q.get());
        ASSERT_EQ(0xbadb095, q.get());
        ASSERT_EQ(0x5ea54e11, q.get());

        ASSERT_EQ(0, q.getSize());
        ASSERT_TRUE(q.isEmpty());
        ASSERT_FALSE(q.isFull());
    }

    TEST_F(QueueTest, Wrap) {
        Queue<std::uint32_t, 4> q{};

        q.put(0xc01bead5);
        q.put(0xcab0b5);
        q.put(0xbadb095);
        q.put(0x5ea54e11);

        ASSERT_EQ(4, q.getSize());
        ASSERT_TRUE(q.isFull());
        ASSERT_FALSE(q.isEmpty());

        ASSERT_EQ(0xc01bead5, q.get());
        ASSERT_EQ(0xcab0b5, q.get());
        ASSERT_EQ(0xbadb095, q.get());
        ASSERT_EQ(0x5ea54e11, q.get());

        ASSERT_EQ(0, q.getSize());
        ASSERT_TRUE(q.isEmpty());
        ASSERT_FALSE(q.isFull());


        q.put(0);
        q.put(1);
        q.put(2);
        q.put(3);

        ASSERT_EQ(0, q.get());
        ASSERT_EQ(1, q.get());
        ASSERT_EQ(2, q.get());
        ASSERT_EQ(3, q.get());
    }

}