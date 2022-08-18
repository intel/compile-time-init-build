#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <core/mock/MockLogger.hpp>

#include <container/PriorityQueue.hpp>

#include <cstdint>

using ::testing::_;

class PriorityQueueTest : public ::testing::Test {
protected:
    MockLogger logger;

    void SetUp() override {
        mockLoggerPtr = &logger;
        EXPECT_CALL(logger, fatal(_)).Times(0);
    }
};

TEST_F(PriorityQueueTest, EmptyQueueSize) {
    PriorityQueue<std::uint32_t, 8> queue;
    ASSERT_EQ(0, queue.size());
}

/*
TEST_F(PriorityQueueTest, EmptyQueueEmpty) {
    PriorityQueue<std::uint32_t, 8> queue;
    ASSERT_TRUE(queue.empty());

    queue.push(32);
    ASSERT_FALSE(queue.empty());
}

TEST_F(PriorityQueueTest, QueueAdd) {
    PriorityQueue<std::uint32_t, 8> queue;

    queue.push(32);

    ASSERT_EQ(1, queue.size());
}

TEST_F(PriorityQueueTest, QueueAddPop) {
    PriorityQueue<std::uint32_t, 8> queue;

    queue.push(56);

    ASSERT_EQ(56, queue.pop());
    ASSERT_EQ(0, queue.size());
}

TEST_F(PriorityQueueTest, QueueAdd2InOrderPop) {
    PriorityQueue<std::uint32_t, 8> queue;

    queue.push(2);
    queue.push(1);

    ASSERT_EQ(2, queue.size());
    ASSERT_EQ(1, queue.top());
    ASSERT_EQ(1, queue.pop());
    ASSERT_EQ(2, queue.pop());
    ASSERT_EQ(0, queue.size());
}

TEST_F(PriorityQueueTest, QueueAdd2OutOfOrderPop) {
    PriorityQueue<std::uint32_t, 8> queue;

    queue.push(1);
    queue.push(2);

    ASSERT_EQ(1, queue.pop());
    ASSERT_EQ(2, queue.pop());
}

TEST_F(PriorityQueueTest, QueueAdd4OutOfOrderPop) {
    PriorityQueue<std::uint32_t, 8> queue;

    queue.push(20);
    queue.push(10);

    ASSERT_EQ(10, queue.pop());
    queue.push(30);
    queue.push(15);

    ASSERT_EQ(15, queue.pop());
    ASSERT_EQ(20, queue.pop());
    ASSERT_EQ(30, queue.pop());
}

TEST_F(PriorityQueueTest, QueueTopEmptyFatal) {
    PriorityQueue<std::uint32_t, 8> queue;

    EXPECT_CALL(logger, fatal(_));

    (void) queue.top();
}

TEST_F(PriorityQueueTest, QueuePushAlmostFull) {
    PriorityQueue<std::uint32_t, 2> queue;

    queue.push(0);
    queue.push(1);
}

TEST_F(PriorityQueueTest, QueuePushFullFatal) {
    PriorityQueue<std::uint32_t, 2> queue;

    EXPECT_CALL(logger, fatal(_));

    queue.push(0);
    queue.push(1);
    queue.push(2);
}

TEST_F(PriorityQueueTest, QueuePopEmptyFatal) {
    PriorityQueue<std::uint32_t, 8> queue;

    EXPECT_CALL(logger, fatal(_));

    queue.pop();
}

TEST_F(PriorityQueueTest, MinSize) {
    PriorityQueue<std::uint32_t, 1> queue;

    queue.push(3);
    ASSERT_TRUE(queue.full());
    ASSERT_EQ(3, queue.pop());
    ASSERT_TRUE(queue.empty());
}
*/