#include <catch2/catch_test_macros.hpp>

#include <container/PriorityQueue.hpp>

#include <cstdint>

namespace {
    TEST_CASE("EmptyQueueSize", "[priority_queue]") {
        PriorityQueue<std::uint32_t, 8> queue;
        REQUIRE(0 == queue.size());
    }

    TEST_CASE("EmptyQueueEmpty", "[priority_queue]") {
        PriorityQueue<std::uint32_t, 8> queue;
        REQUIRE(queue.empty());
    
        queue.push(32);
        REQUIRE_FALSE(queue.empty());
    }

    TEST_CASE("QueueAdd", "[priority_queue]") {
        PriorityQueue<std::uint32_t, 8> queue;
    
        queue.push(32);
    
        REQUIRE(1 == queue.size());
    }
    
    TEST_CASE("QueueAddPop", "[priority_queue]") {
        PriorityQueue<std::uint32_t, 8> queue;
    
        queue.push(56);
    
        REQUIRE(56 == queue.pop());
        REQUIRE(0 == queue.size());
    }
    
    TEST_CASE("QueueAdd2InOrderPop", "[priority_queue]") {
        PriorityQueue<std::uint32_t, 8> queue;
    
        queue.push(2);
        queue.push(1);
    
        REQUIRE(2 == queue.size());
        REQUIRE(1 == queue.top());
        REQUIRE(1 == queue.pop());
        REQUIRE(2 == queue.pop());
        REQUIRE(0 == queue.size());
    }
    
    TEST_CASE("QueueAdd2OutOfOrderPop", "[priority_queue]") {
        PriorityQueue<std::uint32_t, 8> queue;
    
        queue.push(1);
        queue.push(2);
    
        REQUIRE(1 == queue.pop());
        REQUIRE(2 == queue.pop());
    }
    
    TEST_CASE("QueueAdd4OutOfOrderPop", "[priority_queue]") {
        PriorityQueue<std::uint32_t, 8> queue;
    
        queue.push(20);
        queue.push(10);
    
        REQUIRE(10 == queue.pop());
        queue.push(30);
        queue.push(15);
    
        REQUIRE(15 == queue.pop());
        REQUIRE(20 == queue.pop());
        REQUIRE(30 == queue.pop());
    }
    
    TEST_CASE("QueueTopEmptyFatal", "[priority_queue]") {
        PriorityQueue<std::uint32_t, 8> queue;
    
        (void) queue.top();
    }
    
    TEST_CASE("QueuePushAlmostFull", "[priority_queue]") {
        PriorityQueue<std::uint32_t, 2> queue;
    
        queue.push(0);
        queue.push(1);
    }
    
    TEST_CASE("QueuePushFullFatal", "[priority_queue]") {
        PriorityQueue<std::uint32_t, 2> queue;
    
        queue.push(0);
        queue.push(1);
        queue.push(2);
    }
    
    TEST_CASE("QueuePopEmptyFatal", "[priority_queue]") {
        PriorityQueue<std::uint32_t, 8> queue;
    
        queue.pop();
    }
    
    TEST_CASE("MinSize", "[priority_queue]") {
        PriorityQueue<std::uint32_t, 1> queue;
    
        queue.push(3);
        REQUIRE(queue.full());
        REQUIRE(3 == queue.pop());
        REQUIRE(queue.empty());
    }
}