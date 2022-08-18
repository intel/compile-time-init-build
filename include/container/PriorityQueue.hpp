#pragma once

#include <Logger.hpp>
#include <container/Array.hpp>

#include <cstddef>
#include <functional>
#include <algorithm>


/**
 * A PriorityQueue is a container that maintains the highest priority element
 * at the top, and automatically updates it whenever a new element is added.
 * Accessing the top occurs in O(1) time, while removing the top or inserting
 * elements occurs in O(log(n)) time.
 *
 * @tparam ValueType
 *      The element type.
 *
 * @tparam MaxSize
 *      The maximum number of elements the container can hold.
 *
 * @tparam Compare
 *      The comparison function to use.
 */
template<
    typename ValueType,
    std::size_t MaxSize,
    typename Compare = std::greater<ValueType>>
class PriorityQueue {
    static_assert(MaxSize > 0, "PriorityQueue MaxSize must be at least 1");

private:
    using StorageType = ValueType[MaxSize];

    StorageType heap;
    decltype(std::begin(heap)) end;


public:
    /**
     * Create an empty PriorityQueue.
     */
    constexpr PriorityQueue() :
        heap{},
        end{std::begin(heap)}
    {
        // pass
    }

    /**
     * @return Current number of elements.
     */
    [[nodiscard]] constexpr std::size_t size() const {
        return end - std::cbegin(heap);
    }


    /**
     * @return Value of the top or "highest priority" element.
     */
    [[nodiscard]] constexpr ValueType top() const {
        if (empty()) {
            FATAL("PriorityQueue::top() called on empty PriorityQueue");

            return {};

        } else {
            return heap[0];
        }
    }

    /**
     * @return True if the PriorityQueue is empty.
     */
    [[nodiscard]] constexpr bool empty() const {
        return std::cbegin(heap) == end;
    }

    /**
     * @return True if the PriorityQueue can contain no more elements.
     */
    [[nodiscard]] constexpr bool full() const {
        return std::cend(heap) == end;
    }

    /**
     * Push a new value onto the PriorityQueue.
     *
     * If the PriorityQueue is full, it will report a fatal error. Completes
     * in O(log(n)) time where n is the current size of the queue.
     *
     * @param value New value to push into the PriorityQueue.
     */
    constexpr void push(const ValueType value) {
        if (full()) {
            FATAL("PriorityQueue::push() attempted when full");

        } else {
            *end = value;
            end++;
            std::push_heap(std::begin(heap), end, Compare());
        }
    }

    /**
     * Removes and returns the top element of the PriorityQueue.
     *
     * The top element is the highest priority element in the queue. If the
     * PriorityQueue is empty, then a fatal error is reported and a default-
     * constructed ValueType is returned. Completes in O(log(n)) time where n
     * is the current size of the queue.
     *
     * @return The highest priority element in the queue.
     */
    constexpr ValueType pop() {
        if (empty()) {
            FATAL("PriorityQueue::pop() attempted when empty");
            return {};

        } else {
            ValueType topValue = top();
            std::pop_heap(std::begin(heap), end, Compare());
            end--;
            return topValue;
        }
    }
};

