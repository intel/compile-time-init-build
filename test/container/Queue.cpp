#include <container/Queue.hpp>

#include <catch2/catch_test_macros.hpp>

#include "log.hpp"

namespace {
TEST_CASE("EmptyQueue", "[queue]") {
    const Queue<uint32_t, 3> queue({});
    CHECK(0u == queue.get_size());
    CHECK(true == queue.is_empty());
    CHECK(false == queue.is_full());
}

TEST_CASE("APIChecks", "[queue]") {
    Queue<uint32_t, 6> queue;
    queue.put(6u);
    queue.put(0x13);
    queue.put(0x8);
    queue.put(0x43);
    queue.put(0x1024);
    queue.put(0xdeadbeef);

    CHECK(6 == queue.get_size());
    CHECK(true == queue.is_full());
    CHECK(false == queue.is_empty());

    CHECK(6u == queue.get());
    CHECK(0x13 == queue.get());
    CHECK(0x8 == queue.get());
    CHECK(0x43 == queue.get());
    CHECK(0x1024 == queue.get());
    CHECK(0xdeadbeef == queue.get());
    
    CHECK(0 == queue.get_size());
    CHECK(false == queue.is_full());
    CHECK(true == queue.is_empty());
}

TEST_CASE("OverwriteCheck", "[queue]") {
    Queue<uint32_t, 6> queue;
    queue.put(6u);
    queue.put(0x13);
    queue.put(0x8);
    queue.put(0x43);
    queue.put(0x1024);
    queue.put(0xdeadbeef);

    CHECK(6 == queue.get_size());
    CHECK(true == queue.is_full());
    CHECK(false == queue.is_empty());

    queue.put(600u);

    CHECK(6 == queue.get_size());
    CHECK(true == queue.is_full());
    CHECK(false == queue.is_empty());

    CHECK(600u == queue.get());

    CHECK(5 == queue.get_size());
    CHECK(false == queue.is_full());
    CHECK(false == queue.is_empty());
}

TEST_CASE("SizeCheck", "[queue]") {
    Queue<uint32_t, 6> queue;
    
    //Size check after put()
    queue.put(6u);
    CHECK(1 == queue.get_size());
    CHECK(false == queue.is_full());
    CHECK(false == queue.is_empty());

    queue.put(0x13);
    CHECK(2 == queue.get_size());
    CHECK(false == queue.is_full());
    CHECK(false == queue.is_empty());

    queue.put(0x8);
    CHECK(3 == queue.get_size());
    CHECK(false == queue.is_full());
    CHECK(false == queue.is_empty());

    queue.put(0x43);
    CHECK(4 == queue.get_size());
    CHECK(false == queue.is_full());
    CHECK(false == queue.is_empty());

    queue.put(0x1024);
    CHECK(5 == queue.get_size());
    CHECK(false == queue.is_full());
    CHECK(false == queue.is_empty());

    queue.put(0xdeadbeef);
    CHECK(6 == queue.get_size());
    CHECK(true == queue.is_full());
    CHECK(false == queue.is_empty());

    //Size check after get()
    CHECK(6u == queue.get());
    CHECK(5 == queue.get_size());
    CHECK(false == queue.is_full());
    CHECK(false == queue.is_empty());

    CHECK(0x13 == queue.get());
    CHECK(4 == queue.get_size());
    CHECK(false == queue.is_full());
    CHECK(false == queue.is_empty());

    CHECK(0x8 == queue.get());
    CHECK(3 == queue.get_size());
    CHECK(false == queue.is_full());
    CHECK(false == queue.is_empty());

    CHECK(0x43 == queue.get());
    CHECK(2 == queue.get_size());
    CHECK(false == queue.is_full());
    CHECK(false == queue.is_empty());

    CHECK(0x1024 == queue.get());
    CHECK(1 == queue.get_size());
    CHECK(false == queue.is_full());
    CHECK(false == queue.is_empty());

    CHECK(0xdeadbeef == queue.get());
    CHECK(0 == queue.get_size());
    CHECK(false == queue.is_full());
    CHECK(true == queue.is_empty());
}

} // namespace
