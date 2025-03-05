#include <log/fmt/logger.hpp>
#include <msg/callback.hpp>
#include <msg/field.hpp>
#include <msg/handler.hpp>
#include <msg/message.hpp>

#include <stdx/tuple.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <iterator>
#include <string>

namespace {
using namespace msg;

bool dispatched = false;

using id_field = field<"id", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;
using field1 = field<"f1", std::uint32_t>::located<at{0_dw, 15_msb, 0_lsb}>;
using field2 = field<"f2", std::uint32_t>::located<at{1_dw, 23_msb, 16_lsb}>;
using field3 = field<"f3", std::uint32_t>::located<at{1_dw, 15_msb, 0_lsb}>;

using msg_defn = message<"msg", id_field, field1, field2, field3>;

template <auto V> constexpr auto id_match = msg::equal_to_t<id_field, V>{};

std::string log_buffer{};
} // namespace

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(log_buffer)};

TEST_CASE("is_match is true for a match", "[handler]") {
    auto callback = msg::callback<"cb", msg_defn>(
        id_match<0x80>, [](msg::const_view<msg_defn>) { dispatched = true; });
    auto const msg = std::array{0x8000ba11u, 0x0042d00du};

    auto callbacks = stdx::make_tuple(callback);
    auto handler = msg::handler<decltype(callbacks), decltype(msg)>{callbacks};
    CHECK(handler.is_match(msg));
}

TEST_CASE("dispatch single callback (match, raw data)", "[handler]") {
    auto callback = msg::callback<"cb", msg_defn>(
        id_match<0x80>, [](msg::const_view<msg_defn>) { dispatched = true; });
    auto const msg = std::array{0x8000ba11u, 0x0042d00du};

    auto callbacks = stdx::make_tuple(callback);
    auto handler = msg::handler<decltype(callbacks), decltype(msg)>{callbacks};
    dispatched = false;
    CHECK(handler.handle(msg));
    CHECK(dispatched);
}

TEST_CASE("dispatch single callback (match, typed data)", "[handler]") {
    auto callback = msg::callback<"cb", msg_defn>(
        id_match<0x80>, [](msg::const_view<msg_defn>) { dispatched = true; });
    auto const msg = msg::owning<msg_defn>{"id"_field = 0x80};

    auto callbacks = stdx::make_tuple(callback);
    auto handler = msg::handler<decltype(callbacks), decltype(msg)>{callbacks};
    dispatched = false;
    CHECK(handler.handle(msg));
    CHECK(dispatched);
}

TEST_CASE("dispatch single callback (no match)", "[handler]") {
    auto callback = msg::callback<"cb", msg_defn>(
        id_match<0x80>, [](msg::const_view<msg_defn>) { dispatched = true; });
    auto const msg = std::array{0x8100ba11u, 0x0042d00du};

    auto callbacks = stdx::make_tuple(callback);
    auto handler = msg::handler<decltype(callbacks), decltype(msg)>{callbacks};
    dispatched = false;
    CHECK(not handler.handle(msg));
    CHECK(not dispatched);
}

TEST_CASE("log mismatch when no match", "[handler]") {
    auto const msg = std::array{0x8000ba11u, 0x0042d00du};
    auto callbacks = stdx::tuple{};
    auto handler = msg::handler<decltype(callbacks), decltype(msg)>{callbacks};
    CHECK(not handler.handle(msg));
    CAPTURE(log_buffer);
    CHECK(log_buffer.find(
              "None of the registered callbacks (0) claimed this message") !=
          std::string::npos);
}

TEST_CASE("match and dispatch only one callback", "[handler]") {
    auto callback1 = msg::callback<"cb1", msg_defn>(
        id_match<0x80>, [](msg::const_view<msg_defn>) { CHECK(false); });
    auto callback2 = msg::callback<"cb2", msg_defn>(
        id_match<0x44>, [](msg::const_view<msg_defn>) { dispatched = true; });
    auto const msg = std::array{0x4400ba11u, 0x0042d00du};

    auto callbacks = stdx::make_tuple(callback1, callback2);
    static auto handler =
        msg::handler<decltype(callbacks), decltype(msg)>{callbacks};

    dispatched = false;
    CHECK(handler.handle(msg));
    CHECK(dispatched);
}

TEST_CASE("dispatch all callbacks that match", "[handler]") {
    int count{};

    auto callback1 = msg::callback<"cb1", msg_defn>(
        id_match<0x80>, [](msg::const_view<msg_defn>) { CHECK(false); });
    auto callback2 = msg::callback<"cb2", msg_defn>(
        id_match<0x44>, [&](msg::const_view<msg_defn>) { ++count; });
    auto callback3 = msg::callback<"cb3", msg_defn>(
        id_match<0x44>, [&](msg::const_view<msg_defn>) { ++count; });
    auto const msg = std::array{0x4400ba11u, 0x0042d00du};

    auto callbacks = stdx::make_tuple(callback1, callback2, callback3);
    static auto handler =
        msg::handler<decltype(callbacks), decltype(msg)>{callbacks};

    CHECK(handler.handle(msg));
    CHECK(count == 2);
}

namespace {
template <typename T>
using uint8_view =
    typename T::template view_t<typename T::access_t::template span_t<uint8_t>>;
template <typename T>
using const_uint8_view = typename T::template view_t<
    typename T::access_t::template span_t<uint8_t const>>;
} // namespace

TEST_CASE("match and dispatch only one callback with uint8_t storage",
          "[handler]") {
    auto callback1 = msg::callback<"cb1", msg_defn>(
        id_match<0x80>, [&](auto) { CHECK(false); });
    auto callback2 = msg::callback<"cb2", msg_defn>(
        id_match<0x44>, [](auto) { dispatched = true; });
    auto const msg = std::array<uint8_t, 8>{0x00u, 0xbau, 0x11u, 0x44u,
                                            0x00u, 0x42u, 0xd0u, 0x0du};

    auto callbacks = stdx::make_tuple(callback1, callback2);
    static auto handler =
        msg::handler<decltype(callbacks), decltype(msg)>{callbacks};

    dispatched = false;
    handler.handle(msg);
    CHECK(dispatched);
}

TEST_CASE("dispatch with extra args", "[handler]") {
    auto callback = msg::callback<"cb", msg_defn>(
        id_match<0x80>, [](msg::const_view<msg_defn>, int value) {
            dispatched = true;
            CHECK(value == 0xcafe);
        });
    auto const msg = std::array{0x8000ba11u, 0x0042d00du};

    auto callbacks = stdx::make_tuple(callback);
    static auto handler =
        msg::handler<decltype(callbacks), decltype(msg), int>{callbacks};

    dispatched = false;
    CHECK(handler.handle(msg, 0xcafe));
    CHECK(dispatched);
}
