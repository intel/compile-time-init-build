#include <log/fmt/logger.hpp>
#include <msg/callback.hpp>
#include <msg/field.hpp>
#include <msg/message.hpp>

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

using msg_defn =
    message<"msg", id_field::WithRequired<0x80>, field1, field2, field3>;

std::string log_buffer{};
} // namespace

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(log_buffer)};

TEST_CASE("callback matches message", "[handler]") {
    auto callback = msg::callback<msg_defn>("cb"_sc, match::always, [] {});
    auto const msg_match = std::array{0x8000ba11u, 0x0042d00du};
    CHECK(callback.is_match(msg_match));
}

TEST_CASE("callback matches message (alternative range)", "[handler]") {
    auto callback = msg::callback<msg_defn>("cb"_sc, match::always, [] {});
    auto const msg_match = std::array<std::uint8_t, 8>{0x11, 0xba, 0x00, 0x80,
                                                       0x0d, 0xd0, 0x42, 0x00};
    CHECK(callback.is_match(msg_match));
}

TEST_CASE("callback matches message (typed message)", "[handler]") {
    auto callback = msg::callback<msg_defn>("cb"_sc, match::always, [] {});
    auto const msg_match = msg::owning<msg_defn>{"id"_field = 0x80};
    CHECK(callback.is_match(msg_match));
}

TEST_CASE("callback logs mismatch (raw)", "[handler]") {
    auto callback = msg::callback<msg_defn>("cb"_sc, match::always, [] {});
    auto const msg_nomatch = std::array{0x8100ba11u, 0x0042d00du};
    CHECK(not callback.is_match(msg_nomatch));

    log_buffer.clear();
    callback.log_mismatch(msg_nomatch);
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("cb - F:(id (0x81) == 0x80)") != std::string::npos);
}

TEST_CASE("callback logs mismatch (typed)", "[handler]") {
    auto callback = msg::callback<msg_defn>("cb"_sc, match::always, [] {});
    auto const msg_nomatch = msg::owning<msg_defn>{"id"_field = 0x81};
    CHECK(not callback.is_match(msg_nomatch));

    log_buffer.clear();
    callback.log_mismatch(msg_nomatch);
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("cb - F:(id (0x81) == 0x80)") != std::string::npos);
}

TEST_CASE("callback handles message (raw)", "[handler]") {
    auto callback = msg::callback<msg_defn>(
        ""_sc, match::always,
        [](msg::const_view<msg_defn>) { dispatched = true; });
    auto const msg_match = std::array{0x8000ba11u, 0x0042d00du};

    dispatched = false;
    CHECK(callback.handle(msg_match));
    CHECK(dispatched);
}

TEST_CASE("callback handles message (typed)", "[handler]") {
    auto callback = msg::callback<msg_defn>(
        ""_sc, match::always,
        [](msg::const_view<msg_defn>) { dispatched = true; });
    auto const msg_match = msg::owning<msg_defn>{"id"_field = 0x80};

    dispatched = false;
    CHECK(callback.handle(msg_match));
    CHECK(dispatched);
}

TEST_CASE("callback logs match", "[handler]") {
    auto callback = msg::callback<msg_defn>(
        "cb"_sc, match::always,
        [](msg::const_view<msg_defn>) { dispatched = true; });
    auto const msg_match = std::array{0x8000ba11u, 0x0042d00du};

    log_buffer.clear();
    CHECK(callback.handle(msg_match));
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("matched [cb], because [id == 0x80]") !=
          std::string::npos);
}
