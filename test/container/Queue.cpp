#include <catch2/catch_test_macros.hpp>

#include <container/Queue.hpp>


namespace {
    TEST_CASE("EmptyQueueSize", "[queue]") {
        const Queue<std::uint32_t, 4> q{};
        REQUIRE(0 == q.getSize());
        REQUIRE(q.isEmpty());
        REQUIRE_FALSE(q.isFull());
    }

    TEST_CASE("PutOneSize", "[queue]") {
        Queue<std::uint32_t, 4> q{};

        q.put(10);

        REQUIRE(1 == q.getSize());
        REQUIRE_FALSE(q.isEmpty());
        REQUIRE_FALSE(q.isFull());
    }

    TEST_CASE("PutOneGetOne", "[queue]") {
        Queue<std::uint32_t, 4> q{};

        q.put(0xc01bead5);

        REQUIRE(0xc01bead5 == q.get());
        REQUIRE(0 == q.getSize());
        REQUIRE(q.isEmpty());
        REQUIRE_FALSE(q.isFull());
    }

    TEST_CASE("Full", "[queue]") {
        Queue<std::uint32_t, 4> q{};

        q.put(0xc01bead5);
        q.put(0xcab0b5);
        q.put(0xbadb095);
        q.put(0x5ea54e11);

        REQUIRE(4 == q.getSize());
        REQUIRE(q.isFull());
        REQUIRE_FALSE(q.isEmpty());

        REQUIRE(0xc01bead5 == q.get());
        REQUIRE(0xcab0b5 == q.get());
        REQUIRE(0xbadb095 == q.get());
        REQUIRE(0x5ea54e11 == q.get());

        REQUIRE(0 == q.getSize());
        REQUIRE(q.isEmpty());
        REQUIRE_FALSE(q.isFull());
    }

    TEST_CASE("Wrap", "[queue]") {
        Queue<std::uint32_t, 4> q{};

        q.put(0xc01bead5);
        q.put(0xcab0b5);
        q.put(0xbadb095);
        q.put(0x5ea54e11);

        REQUIRE(4 == q.getSize());
        REQUIRE(q.isFull());
        REQUIRE_FALSE(q.isEmpty());

        REQUIRE(0xc01bead5 == q.get());
        REQUIRE(0xcab0b5 == q.get());
        REQUIRE(0xbadb095 == q.get());
        REQUIRE(0x5ea54e11 == q.get());

        REQUIRE(0 == q.getSize());
        REQUIRE(q.isEmpty());
        REQUIRE_FALSE(q.isFull());


        q.put(0);
        q.put(1);
        q.put(2);
        q.put(3);

        REQUIRE(0 == q.get());
        REQUIRE(1 == q.get());
        REQUIRE(2 == q.get());
        REQUIRE(3 == q.get());
    }

}