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

using msg_defn = message<"msg", id_field, field1, field2, field3>;

constexpr auto id_match = msg::equal_to<id_field, 0x80>;

constexpr struct custom_match_t {
    using is_matcher = void;

    [[nodiscard]] constexpr auto operator()(msg::owning<msg_defn>) const {
        return true;
    }

    [[nodiscard]] constexpr static auto describe() { return "custom"_sc; }

    [[nodiscard]] constexpr static auto describe_match(msg::owning<msg_defn>) {
        return describe();
    }
} custom_match;

std::string log_buffer{};
} // namespace

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(log_buffer)};

TEST_CASE("callback matches message by view", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(id_match, [] {});
    auto const msg_match = std::array{0x8000ba11u, 0x0042d00du};
    CHECK(callback.is_match(msg_match));
}

TEST_CASE("callback matches message by owning", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(custom_match, [] {});
    auto const msg_match = std::array{0x8000ba11u, 0x0042d00du};
    CHECK(callback.is_match(msg_match));
}

TEST_CASE("callback matches message (alternative range)", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(id_match, [] {});
    auto const msg_match = std::array<std::uint8_t, 8>{0x11, 0xba, 0x00, 0x80,
                                                       0x0d, 0xd0, 0x42, 0x00};
    CHECK(callback.is_match(msg_match));
}

TEST_CASE("callback matches message (typed message)", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(id_match, [] {});
    auto const msg_match = msg::owning<msg_defn>{"id"_field = 0x80};
    CHECK(callback.is_match(msg_match));
}

TEST_CASE("callback logs mismatch by view (raw)", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(id_match, [] {});
    auto const msg_nomatch = std::array{0x8100ba11u, 0x0042d00du};
    CHECK(not callback.is_match(msg_nomatch));

    log_buffer.clear();
    callback.log_mismatch(msg_nomatch);
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("cb - F:(id (0x81) == 0x80)") != std::string::npos);
}

TEST_CASE("callback logs mismatch by owning (raw)", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(custom_match, [] {});
    auto const msg_nomatch = std::array{0x8100ba11u, 0x0042d00du};

    log_buffer.clear();
    callback.log_mismatch(msg_nomatch);
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("cb - F:(custom)") != std::string::npos);
}

TEST_CASE("callback logs mismatch (typed)", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(id_match, [] {});
    auto const msg_nomatch = msg::owning<msg_defn>{"id"_field = 0x81};
    CHECK(not callback.is_match(msg_nomatch));

    log_buffer.clear();
    callback.log_mismatch(msg_nomatch);
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("cb - F:(id (0x81) == 0x80)") != std::string::npos);
}

TEST_CASE("callback handles message by view (raw)", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(
        id_match, [](msg::const_view<msg_defn>) { dispatched = true; });
    auto const msg_match = std::array{0x8000ba11u, 0x0042d00du};

    dispatched = false;
    CHECK(callback.handle(msg_match));
    CHECK(dispatched);
}

TEST_CASE("callback handles message by owning (raw)", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(
        id_match, [](msg::owning<msg_defn>) { dispatched = true; });
    auto const msg_match = std::array{0x8000ba11u, 0x0042d00du};

    dispatched = false;
    CHECK(callback.handle(msg_match));
    CHECK(dispatched);
}

namespace {
template <typename T>
using uint8_view =
    typename T::template view_t<typename T::access_t::template span_t<uint8_t>>;
template <typename T>
using const_uint8_view = typename T::template view_t<
    typename T::access_t::template span_t<uint8_t const>>;
} // namespace

TEST_CASE("callback handles message (custom raw format)", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(
        id_match, [](auto) { dispatched = true; });
    auto const msg_match = std::array<uint8_t, 32>{0x00u, 0xbau, 0x11u, 0x80u,
                                                   0x00u, 0x42u, 0xd0u, 0x0du};

    dispatched = false;
    CHECK(callback.handle(msg_match));
    CHECK(dispatched);
}

TEST_CASE("callback handles message (typed)", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(
        id_match, [](auto) { dispatched = true; });
    auto const msg_match = msg::owning<msg_defn>{"id"_field = 0x80};

    dispatched = false;
    CHECK(callback.handle(msg_match));
    CHECK(dispatched);
}

TEST_CASE("callback logs match as INFO", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(
        id_match, [](msg::const_view<msg_defn>) { dispatched = true; });
    auto const msg_match = std::array{0x8000ba11u, 0x0042d00du};

    log_buffer.clear();
    CHECK(callback.handle(msg_match));
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("matched [cb], because [id == 0x80]") !=
          std::string::npos);
    CHECK(log_buffer.find("INFO") != std::string::npos);
}

TEST_CASE("callback logs match using message environment", "[callback]") {
    using trace_msg_defn = msg_defn::with_env<
        stdx::make_env_t<logging::get_level, logging::level::TRACE>>;
    auto callback = msg::callback<"cb", trace_msg_defn>(
        id_match, [](msg::const_view<trace_msg_defn>) { dispatched = true; });
    auto const msg_match = std::array{0x8000ba11u, 0x0042d00du};

    log_buffer.clear();
    CHECK(callback.handle(msg_match));
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("matched [cb], because [id == 0x80]") !=
          std::string::npos);
    CHECK(log_buffer.find("TRACE") != std::string::npos);
}

TEST_CASE("callback with convenience matcher", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(
        msg::equal_to<id_field, 0x80>,
        [](msg::const_view<msg_defn>) { dispatched = true; });
    auto const msg_match = msg::owning<msg_defn>{"id"_field = 0x80};

    dispatched = false;
    CHECK(callback.handle(msg_match));
    CHECK(dispatched);
}

TEST_CASE("callback with compound convenience matcher", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(
        msg::equal_to<id_field, 0x80> and msg::equal_to<field1, 11>,
        [](msg::const_view<msg_defn>) { dispatched = true; });
    auto const msg_match =
        msg::owning<msg_defn>{"id"_field = 0x80, "f1"_field = 11};

    dispatched = false;
    CHECK(callback.handle(msg_match));
    CHECK(dispatched);
}

TEST_CASE("alternative matcher syntax (equality)", "[callback]") {
    auto callback =
        msg::callback<"cb", msg_defn>("id"_field == msg::constant<0x80>, [] {});
    auto const msg_match = std::array{0x8000ba11u, 0x0042d00du};
    CHECK(callback.is_match(msg_match));
}

namespace {
template <typename RelOp, auto V> struct expect_matcher_t {
    template <typename Field>
    constexpr auto operator()(msg::rel_matcher_t<RelOp, Field, V>) const {
        return true;
    }
    constexpr auto operator()(auto) const { return false; }
};
template <typename RelOp, auto V>
constexpr auto expect_matcher = expect_matcher_t<RelOp, V>{};
} // namespace

TEST_CASE("alternative matcher syntax (inequality)", "[callback]") {
    auto callback =
        msg::callback<"cb", msg_defn>("id"_field != msg::constant<0x80>, [] {});
    static_assert(expect_matcher<std::not_equal_to<>, 0x80>(
        typename decltype(callback)::matcher_t{}));
}

TEST_CASE("alternative matcher syntax (less)", "[callback]") {
    auto callback =
        msg::callback<"cb", msg_defn>("id"_field < msg::constant<0x80>, [] {});
    static_assert(expect_matcher<std::less<>, 0x80>(
        typename decltype(callback)::matcher_t{}));
}

TEST_CASE("alternative matcher syntax (less than or equal)", "[callback]") {
    auto callback =
        msg::callback<"cb", msg_defn>("id"_field <= msg::constant<0x80>, [] {});
    static_assert(expect_matcher<std::less_equal<>, 0x80>(
        typename decltype(callback)::matcher_t{}));
}

TEST_CASE("alternative matcher syntax (greater)", "[callback]") {
    auto callback =
        msg::callback<"cb", msg_defn>("id"_field > msg::constant<0x80>, [] {});
    static_assert(expect_matcher<std::greater<>, 0x80>(
        typename decltype(callback)::matcher_t{}));
}

TEST_CASE("alternative matcher syntax (greater than or equal)", "[callback]") {
    auto callback =
        msg::callback<"cb", msg_defn>("id"_field >= msg::constant<0x80>, [] {});
    static_assert(expect_matcher<std::greater_equal<>, 0x80>(
        typename decltype(callback)::matcher_t{}));
}

TEST_CASE("alternative matcher syntax (not)", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(
        not("id"_field < msg::constant<0x80>), [] {});
    static_assert(expect_matcher<std::greater_equal<>, 0x80>(
        typename decltype(callback)::matcher_t{}));
}

TEST_CASE("alternative matcher syntax (and)", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(
        "id"_field > msg::constant<0x80> and "id"_field > msg::constant<0x90>,
        [] {});
    static_assert(expect_matcher<std::greater<>, 0x90>(
        typename decltype(callback)::matcher_t{}));
}

TEST_CASE("alternative matcher syntax (or)", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(
        "id"_field > msg::constant<0x80> or "id"_field > msg::constant<0x90>,
        [] {});
    static_assert(expect_matcher<std::greater<>, 0x80>(
        typename decltype(callback)::matcher_t{}));
}

TEST_CASE("alternative matcher syntax (multi-field)", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>(
        "id"_field == msg::constant<0x80> and "f1"_field == msg::constant<1>,
        [] {});
    static_assert(std::same_as<typename decltype(callback)::matcher_t,
                               match::and_t<msg::equal_to_t<id_field, 0x80>,
                                            msg::equal_to_t<field1, 1>>>);
}

TEST_CASE("alternative matcher syntax (value in set)", "[callback]") {
    auto callback =
        msg::callback<"cb", msg_defn>("id"_field.in<0x80, 0x90>, [] {});
    static_assert(std::same_as<typename decltype(callback)::matcher_t,
                               match::or_t<msg::equal_to_t<id_field, 0x80>,
                                           msg::equal_to_t<id_field, 0x90>>>);
}

TEST_CASE("alternative matcher syntax (value in singleton)", "[callback]") {
    auto callback = msg::callback<"cb", msg_defn>("id"_field.in<0x80>, [] {});
    static_assert(std::same_as<typename decltype(callback)::matcher_t,
                               msg::equal_to_t<id_field, 0x80>>);
}

TEST_CASE("alternative matcher syntax (and-combine matcher with matcher maker)",
          "[callback]") {
    auto cb1 = msg::callback<"cb", msg_defn>(
        id_match and "f1"_field == msg::constant<1>,
        [](msg::const_view<msg_defn>) { dispatched = true; });
    auto cb2 = msg::callback<"cb", msg_defn>(
        "f1"_field == msg::constant<1> and id_match,
        [](msg::const_view<msg_defn>) { dispatched = true; });
    auto const msg_match =
        msg::owning<msg_defn>{"id"_field = 0x80, "f1"_field = 1};

    dispatched = false;
    CHECK(cb1.handle(msg_match));
    CHECK(dispatched);
    dispatched = false;
    CHECK(cb2.handle(msg_match));
    CHECK(dispatched);
}

TEST_CASE("alternative matcher syntax (or-combine matcher with matcher maker)",
          "[callback]") {
    auto cb1 = msg::callback<"cb", msg_defn>(
        id_match or "id"_field == msg::constant<0x81>,
        [](msg::const_view<msg_defn>) { dispatched = true; });
    auto cb2 = msg::callback<"cb", msg_defn>(
        "id"_field == msg::constant<0x81> or id_match,
        [](msg::const_view<msg_defn>) { dispatched = true; });
    auto const msg_match = msg::owning<msg_defn>{"id"_field = 0x80};

    dispatched = false;
    CHECK(cb1.handle(msg_match));
    CHECK(dispatched);
    dispatched = false;
    CHECK(cb2.handle(msg_match));
    CHECK(dispatched);
}
