#include "log.hpp"

#include <container/queue.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

TEST_CASE("empty", "[queue]") {
    queue<std::uint32_t, 1> const q({});
    CHECK(0u == q.size());
    CHECK(1u == q.capacity());
    CHECK(q.empty());
    CHECK(not q.full());
}

TEST_CASE("full", "[queue]") {
    queue<std::uint32_t, 1> q({});
    q.push(1u);
    CHECK(1u == q.size());
    CHECK(1u == q.capacity());
    CHECK(not q.empty());
    CHECK(q.full());
}

TEST_CASE("push/pop", "[queue]") {
    queue<std::uint32_t, 6> q;
    q.push(1u);
    CHECK(1u == q.size());
    q.push(2u);
    CHECK(2u == q.size());
    q.push(3u);

    REQUIRE(3u == q.size());
    CHECK(1u == q.pop());
    REQUIRE(2u == q.size());
    CHECK(2u == q.pop());
    REQUIRE(1u == q.size());
    CHECK(3u == q.pop());

    CHECK(0u == q.size());
    CHECK(not q.full());
    CHECK(q.empty());
}

TEST_CASE("push returns reference", "[queue]") {
    queue<std::uint32_t, 1> q;
    auto &ref = q.push(1u);
    ref = 2u;
    CHECK(2u == q.pop());
}

TEST_CASE("wraparound", "[queue]") {
    queue<std::uint32_t, 2> q;

    q.push(1u);
    CHECK(1u == q.size());
    q.push(2u);
    REQUIRE(2u == q.size());
    CHECK(1u == q.pop());
    CHECK(1u == q.size());
    q.push(3u);
    REQUIRE(2u == q.size());
    CHECK(2u == q.pop());
    CHECK(3u == q.pop());

    CHECK(0u == q.size());
    CHECK(not q.full());
    CHECK(q.empty());
}

TEST_CASE("safe over/underflow", "[queue]") {
    queue<std::uint32_t, 1> q;
    q.push(1u);
    CHECK_THROWS_AS(q.push(2u), test_log_config::exception);
    CHECK(1u == q.pop());
    CHECK_THROWS_AS(q.pop(), test_log_config::exception);
    CHECK_THROWS_AS(q.front(), test_log_config::exception);
    CHECK_THROWS_AS(q.back(), test_log_config::exception);
}

namespace {
struct move_only {
    constexpr move_only() = default;
    constexpr move_only(int i) : value{i} {}
    constexpr move_only(move_only &&) = default;
    constexpr auto operator=(move_only &&) noexcept -> move_only & = default;
    int value{};
};
} // namespace

TEST_CASE("move only elements", "[queue]") {
    queue<move_only, 1> q;
    q.push(move_only{1});
    CHECK(1 == q.pop().value);
}

TEST_CASE("front/back", "[queue]") {
    queue<std::uint32_t, 2> q;
    q.push(1u);
    q.push(2u);
    CHECK(q.front() == 1u);
    CHECK(q.back() == 2u);

    q.front() = 2u;
    CHECK(q.front() == 2u);

    q.back() = 3u;
    CHECK(q.back() == 3u);
}
