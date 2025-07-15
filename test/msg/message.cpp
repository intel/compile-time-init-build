#include <log/fmt/logger.hpp>
#include <msg/message.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iterator>
#include <string>
#include <type_traits>

namespace {
using namespace msg;

using id_field = field<"id", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;
using field1 = field<"f1", std::uint32_t>::located<at{0_dw, 15_msb, 0_lsb}>;
using field2 = field<"f2", std::uint32_t>::located<at{1_dw, 23_msb, 16_lsb}>;
using field3 = field<"f3", std::uint32_t>::located<at{1_dw, 15_msb, 8_lsb},
                                                   at{1_dw, 7_msb, 0_lsb}>;

using msg_defn =
    message<"msg", id_field::with_required<0x80>, field1, field2, field3>;
using test_msg = owning<msg_defn>;

std::string log_buffer{};
} // namespace

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(log_buffer)};

TEST_CASE("message with automatic storage", "[message]") {
    test_msg msg{};
    auto data = msg.data();
    STATIC_REQUIRE(
        std::is_same_v<decltype(data), stdx::span<std::uint32_t, 2>>);
}

TEST_CASE("message with custom storage", "[message]") {
    auto arr = std::array<std::uint16_t, 4>{0x0, 0x8000, 0xd00d, 0x0042};
    msg_defn::owner_t msg{arr, "f1"_field = 0xba11};
    CHECK(0x80 == msg.get("id"_field));
    CHECK(0xba11 == msg.get("f1"_field));
    STATIC_REQUIRE(
        std::is_same_v<typename decltype(msg)::storage_t, decltype(arr)>);
}

TEST_CASE("message constructed from span", "[message]") {
    auto const arr =
        typename msg_defn::default_storage_t{0x8000'ba11, 0x0042'd00d};
    auto s = stdx::span{arr};

    msg_defn::owner_t msg{s};
    STATIC_REQUIRE(std::is_same_v<typename decltype(msg)::storage_t,
                                  std::remove_const_t<decltype(arr)>>);
    msg.set("f1"_field = 0xba12);
    CHECK(0x8000'ba11 == arr[0]);
}

TEST_CASE("construct with field values", "[message]") {
    test_msg msg{"f1"_field = 0xba11, "f2"_field = 0x42, "f3"_field = 0xd00d};

    CHECK(0x80 == msg.get("id"_field));
    CHECK(0xba11 == msg.get("f1"_field));
    CHECK(0x42 == msg.get("f2"_field));
    CHECK(0xd00d == msg.get("f3"_field));

    auto const data = msg.data();
    CHECK(0x8000'ba11 == data[0]);
    CHECK(0x0042'd00d == data[1]);
}

TEST_CASE("use field names as template args", "[message]") {
    auto msg = []<auto F>() {
        return test_msg{F = 0xba11};
    }.template operator()<"f1"_field>();

    auto const data = msg.data();
    CHECK(0x8000'ba11 == data[0]);
}

TEST_CASE("construct with field defaults", "[message]") {
    test_msg msg{};

    CHECK(0x80 == msg.get("id"_field));
    CHECK(0x0 == msg.get("f1"_field));
    CHECK(0x0 == msg.get("f2"_field));
    CHECK(0x0 == msg.get("f3"_field));

    auto const data = msg.data();
    CHECK(0x8000'0000 == data[0]);
    CHECK(0x0 == data[1]);
}

TEST_CASE("construct with raw data in range", "[message]") {
    auto arr = typename test_msg::storage_t{0x8000'ba11, 0x0042'd00d};
    test_msg msg{arr};

    CHECK(0x80 == msg.get("id"_field));
    CHECK(0xba11 == msg.get("f1"_field));
    CHECK(0x42 == msg.get("f2"_field));
    CHECK(0xd00d == msg.get("f3"_field));
}

TEST_CASE("construct with raw data in range, some fields set", "[message]") {
    auto arr = typename test_msg::storage_t{0x8000'0000, 0x0042'd00d};
    test_msg msg{arr, "f1"_field = 0xba11};

    CHECK(0xba11 == msg.get("f1"_field));
}

TEST_CASE("view with read-only external storage", "[message]") {
    auto const arr =
        typename msg_defn::default_storage_t{0x8000'ba11, 0x0042'd00d};
    const_view<msg_defn> msg{arr};

    CHECK(0x80 == msg.get("id"_field));
    CHECK(0xba11 == msg.get("f1"_field));
    CHECK(0x42 == msg.get("f2"_field));
    CHECK(0xd00d == msg.get("f3"_field));
}

TEST_CASE("view with external storage (oversized)", "[message]") {
    auto const arr = std::array<std::uint32_t, 8>{0x8000'ba11, 0x0042'd00d};
    msg_defn::view_t msg{arr};
    CHECK(0x80 == msg.get("id"_field));
}

TEST_CASE("view with external storage (alternate value_type)", "[message]") {
    auto const arr =
        std::array<std::uint16_t, 4>{0xba11, 0x8000, 0xd00d, 0x0042};
    msg_defn::view_t msg{arr};
    CHECK(0x80 == msg.get("id"_field));
}

TEST_CASE("view with mutable external storage", "[message]") {
    auto arr = typename msg_defn::default_storage_t{0x8000'ba11, 0x0042'd00d};
    mutable_view<msg_defn> msg{arr, "f2"_field = 0x41};
    CHECK(0x0041'd00d == arr[1]);

    msg.set("f1"_field = 0);
    CHECK(0x8000'0000 == arr[0]);
}

TEST_CASE("view constructed from span (mutable)", "[message]") {
    auto arr = typename msg_defn::default_storage_t{0x8000'ba11, 0x0042'd00d};
    auto s = stdx::span{arr};
    msg_defn::view_t msg{s, "f2"_field = 0x41};

    msg.set("f1"_field = 0);
    CHECK(0x8000'0000 == arr[0]);
}

TEST_CASE("view constructed from span (const)", "[message]") {
    auto const arr =
        typename msg_defn::default_storage_t{0x8000'ba11, 0x0042'd00d};
    auto s = stdx::span{arr};
    msg_defn::view_t msg{s};
    CHECK(0x80 == msg.get("id"_field));
}

TEST_CASE("view constructed from read-only owning message", "[message]") {
    test_msg const msg{"f1"_field = 0xba11, "f2"_field = 0x42,
                       "f3"_field = 0xd00d};
    msg_defn::view_t view{msg};

    CHECK(0x80 == view.get("id"_field));
    CHECK(0xba11 == view.get("f1"_field));
    CHECK(0x42 == view.get("f2"_field));
    CHECK(0xd00d == view.get("f3"_field));
}

TEST_CASE("view constructed from mutable owning message", "[message]") {
    test_msg msg{"f1"_field = 0xba11, "f2"_field = 0x42, "f3"_field = 0xd00d};
    msg_defn::view_t view{msg, "f2"_field = 0x41};
    CHECK(0x41 == msg.get("f2"_field));

    view.set("f1"_field = 0xba12);
    CHECK(0xba12 == msg.get("f1"_field));
}

TEST_CASE("message constructed from view", "[message]") {
    auto const arr =
        typename msg_defn::default_storage_t{0x8000'ba11, 0x0042'd00d};
    msg::const_view<msg_defn> v{arr};

    msg_defn::owner_t msg{v};
    STATIC_REQUIRE(std::is_same_v<typename decltype(msg)::storage_t,
                                  std::remove_const_t<decltype(arr)>>);
    msg.set("f1"_field = 0xba12);
    CHECK(0x8000'ba11 == arr[0]);
}

TEST_CASE("implicit construct view from oversized storage", "[message]") {
    auto const arr = std::array<std::uint32_t, 4>{0x8000'ba11, 0x0042'd00d};
    using view_t = const_view<msg_defn>;

    [](view_t msg) {
        CHECK(0x80 == msg.get("id"_field));
        CHECK(0xba11 == msg.get("f1"_field));
        CHECK(0x42 == msg.get("f2"_field));
        CHECK(0xd00d == msg.get("f3"_field));
    }(arr);
}

TEST_CASE("const view from owning message", "[message]") {
    test_msg msg{};
    auto v = msg.as_const_view();
    STATIC_REQUIRE(
        std::is_same_v<decltype(v.data()),
                       typename test_msg::definition_t::default_const_span_t>);
    msg.set("f1"_field = 0xba11);
    CHECK(0xba11 == v.get("f1"_field));
}

TEST_CASE("mutable view from owning message", "[message]") {
    test_msg msg{};
    auto v = msg.as_mutable_view();
    STATIC_REQUIRE(
        std::is_same_v<decltype(v.data()),
                       typename test_msg::definition_t::default_span_t>);
    v.set("f1"_field = 0xba11);
    CHECK(0xba11 == msg.get("f1"_field));
}

TEST_CASE("owning from view", "[message]") {
    test_msg msg{"f1"_field = 0xba11, "f2"_field = 0x42, "f3"_field = 0xd00d};
    auto v = msg.as_mutable_view();
    auto o = v.as_owning();
    STATIC_REQUIRE(std::is_same_v<decltype(o), test_msg>);
    CHECK(std::equal(msg.data().begin(), msg.data().end(), o.data().begin()));
}

TEST_CASE("const view from mutable view", "[message]") {
    test_msg msg{};
    auto mv = msg.as_mutable_view();
    auto v = mv.as_const_view();
    STATIC_REQUIRE(
        std::is_same_v<decltype(v.data()),
                       typename test_msg::definition_t::default_const_span_t>);
    msg.set("f1"_field = 0xba11);
    CHECK(0xba11 == v.get("f1"_field));
}

TEST_CASE("equal_to matcher", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(msg::equal_to<id_field, 0x80>(msg));
    CHECK(msg::equal_to<field1, 0xba11>(msg));
    CHECK(msg::equal_to<field2, 0x42>(msg));
    CHECK(msg::equal_to<field3, 0xd00d>(msg));

    CHECK(not msg::equal_to<id_field, 0x0>(msg));
    CHECK(not msg::equal_to<field1, 0x0>(msg));
    CHECK(not msg::equal_to<field2, 0x0>(msg));
    CHECK(not msg::equal_to<field3, 0x0>(msg));
}

TEST_CASE("default matcher", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x0, 0x00}};

    CHECK(msg::equal_default<id_field>(msg));
    CHECK(msg::equal_default<field1>(msg));
    CHECK(msg::equal_default<field2>(msg));
    CHECK(msg::equal_default<field3>(msg));
}

TEST_CASE("default matcher (no match)", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(not msg::equal_default<id_field>(msg));
    CHECK(not msg::equal_default<field1>(msg));
    CHECK(not msg::equal_default<field2>(msg));
    CHECK(not msg::equal_default<field3>(msg));
}

TEST_CASE("in matcher", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(msg::in<id_field, 0x80>(msg));
    CHECK(msg::in<field1, 0xba11>(msg));
    CHECK(msg::in<field2, 0x42>(msg));
    CHECK(msg::in<field3, 0xd00d>(msg));

    CHECK(not msg::in<id_field, 0x11>(msg));
    CHECK(not msg::in<field1, 0x1111>(msg));
    CHECK(not msg::in<field2, 0x11>(msg));
    CHECK(not msg::in<field3, 0x1111>(msg));

    CHECK(msg::in<id_field, 0x80, 0x0>(msg));
    CHECK(msg::in<field1, 0xba11, 0x0>(msg));
    CHECK(msg::in<field2, 0x42, 0x0>(msg));
    CHECK(msg::in<field3, 0xd00d, 0x0>(msg));

    CHECK(not msg::in<id_field, 0x11, 0x0>(msg));
    CHECK(not msg::in<field1, 0x1111, 0x0>(msg));
    CHECK(not msg::in<field2, 0x11, 0x0>(msg));
    CHECK(not msg::in<field3, 0x1111, 0x0>(msg));

    CHECK(msg::in<id_field, 0x80, 0x0, 0xE>(msg));
    CHECK(msg::in<field1, 0xba11, 0x0, 0xEEEE>(msg));
    CHECK(msg::in<field2, 0x42, 0x0, 0xEE>(msg));
    CHECK(msg::in<field3, 0xd00d, 0x0, 0xEEEE>(msg));

    CHECK(not msg::in<id_field, 0x11, 0x0, 0xE>(msg));
    CHECK(not msg::in<field1, 0x1111, 0x0, 0xEEEE>(msg));
    CHECK(not msg::in<field2, 0x11, 0x0, 0xEE>(msg));
    CHECK(not msg::in<field3, 0x1111, 0x0, 0xEEEE>(msg));
}

TEST_CASE("greater_than matcher", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(not msg::greater_than<id_field, 0x80>(msg));
    CHECK(not msg::greater_than<field1, 0xba11>(msg));
    CHECK(not msg::greater_than<field2, 0x42>(msg));
    CHECK(not msg::greater_than<field3, 0xd00d>(msg));

    CHECK(msg::greater_than<id_field, 0x11>(msg));
    CHECK(msg::greater_than<field1, 0x1111>(msg));
    CHECK(msg::greater_than<field2, 0x11>(msg));
    CHECK(msg::greater_than<field3, 0x1111>(msg));
}

TEST_CASE("greater_than_or_equal_to matcher", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(not msg::greater_than_or_equal_to<id_field, 0xEE>(msg));
    CHECK(not msg::greater_than_or_equal_to<field1, 0xEEEE>(msg));
    CHECK(not msg::greater_than_or_equal_to<field2, 0xEE>(msg));
    CHECK(not msg::greater_than_or_equal_to<field3, 0xEEEE>(msg));

    CHECK(msg::greater_than_or_equal_to<id_field, 0x80>(msg));
    CHECK(msg::greater_than_or_equal_to<field1, 0xba11>(msg));
    CHECK(msg::greater_than_or_equal_to<field2, 0x42>(msg));
    CHECK(msg::greater_than_or_equal_to<field3, 0xd00d>(msg));

    CHECK(msg::greater_than_or_equal_to<id_field, 0x11>(msg));
    CHECK(msg::greater_than_or_equal_to<field1, 0x1111>(msg));
    CHECK(msg::greater_than_or_equal_to<field2, 0x11>(msg));
    CHECK(msg::greater_than_or_equal_to<field3, 0x1111>(msg));
}

TEST_CASE("less_than matcher", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(not msg::less_than<id_field, 0x80>(msg));
    CHECK(not msg::less_than<field1, 0xba11>(msg));
    CHECK(not msg::less_than<field2, 0x42>(msg));
    CHECK(not msg::less_than<field3, 0xd00d>(msg));

    CHECK(msg::less_than<id_field, 0xEE>(msg));
    CHECK(msg::less_than<field1, 0xEEEE>(msg));
    CHECK(msg::less_than<field2, 0xEE>(msg));
    CHECK(msg::less_than<field3, 0xEEEE>(msg));
}

TEST_CASE("less_than_or_equal_to matcher", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(msg::less_than_or_equal_to<id_field, 0xEE>(msg));
    CHECK(msg::less_than_or_equal_to<field1, 0xEEEE>(msg));
    CHECK(msg::less_than_or_equal_to<field2, 0xEE>(msg));
    CHECK(msg::less_than_or_equal_to<field3, 0xEEEE>(msg));

    CHECK(msg::less_than_or_equal_to<id_field, 0x80>(msg));
    CHECK(msg::less_than_or_equal_to<field1, 0xba11>(msg));
    CHECK(msg::less_than_or_equal_to<field2, 0x42>(msg));
    CHECK(msg::less_than_or_equal_to<field3, 0xd00d>(msg));

    CHECK(not msg::less_than_or_equal_to<id_field, 0x11>(msg));
    CHECK(not msg::less_than_or_equal_to<field1, 0x1111>(msg));
    CHECK(not msg::less_than_or_equal_to<field2, 0x11>(msg));
    CHECK(not msg::less_than_or_equal_to<field3, 0x1111>(msg));
}

TEST_CASE("describe a message", "[message]") {
    test_msg m{"f1"_field = 0xba11, "f2"_field = 0x42, "f3"_field = 0xd00d};
    CIB_INFO("{}", m.describe());
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("msg(f1: 0xba11, id: 0x80, f3: 0xd00d, f2: 0x42)") !=
          std::string::npos);
}

namespace {
using uint8_storage_t = msg_defn::custom_storage_t<std::array, std::uint8_t>;
using test_uint8_msg = msg_defn::owner_t<uint8_storage_t>;
} // namespace

TEST_CASE("construct with 8-bit storage", "[message]") {
    test_uint8_msg msg{"f1"_field = 0xba11, "f2"_field = 0x42,
                       "f3"_field = 0xd00d};

    CHECK(0x80 == msg.get("id"_field));
    CHECK(0xba11 == msg.get("f1"_field));
    CHECK(0x42 == msg.get("f2"_field));
    CHECK(0xd00d == msg.get("f3"_field));

    auto const expected =
        std::array<std::uint8_t, 7>{0x11, 0xba, 0x00, 0x80, 0x0d, 0xd0, 0x42};
    auto const data = msg.data();
    CHECK(std::equal(std::begin(expected), std::end(expected), std::begin(data),
                     std::end(data)));
}

TEST_CASE("view with external custom storage (oversized)", "[message]") {
    auto const arr = std::array<std::uint8_t, 32>{0x00, 0xba, 0x11, 0x80,
                                                  0x00, 0x42, 0xd0, 0x0d};
    msg_defn::view_t msg{arr};
    CHECK(0x80 == msg.get("id"_field));
}

TEST_CASE("implicitly downsize a message view", "[message]") {
    auto const arr = std::array<std::uint32_t, 4>{0x8000'ba11, 0x0042'd00d};
    msg_defn::view_t msg{arr};
    STATIC_REQUIRE(std::is_same_v<decltype(msg.data()),
                                  stdx::span<std::uint32_t const, 4>>);
    const_view<msg_defn> v = msg;
    STATIC_REQUIRE(
        std::is_same_v<decltype(v.data()), stdx::span<std::uint32_t const, 2>>);
    CHECK(msg.data()[0] == v.data()[0]);
    CHECK(msg.data()[1] == v.data()[1]);
}

TEST_CASE("view_of concept", "[message]") {
    auto const arr = std::array<std::uint32_t, 4>{0x8000'ba11, 0x0042'd00d};
    msg_defn::view_t msg{arr};
    STATIC_REQUIRE(msg::view_of<decltype(msg), msg_defn>);

    const_view<msg_defn> v = msg;
    STATIC_REQUIRE(msg::view_of<decltype(v), msg_defn>);
    STATIC_REQUIRE(msg::const_view_of<decltype(v), msg_defn>);
}

TEST_CASE("message fields are canonically sorted by lsb", "[message]") {
    using defn = message<"msg", field2, field1>;
    STATIC_REQUIRE(std::is_same_v<defn, message<"msg", field1, field2>>);
}

TEST_CASE("extend message with more fields", "[message]") {
    using base_defn = message<"msg_base", id_field, field1>;
    using defn = extend<base_defn, "msg", field2, field3>;
    using expected_defn = message<"msg", id_field, field1, field2, field3>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("extend message with duplicate field", "[message]") {
    using base_defn = message<"msg_base", id_field, field1>;
    using defn = extend<base_defn, "msg", field1>;
    using expected_defn = message<"msg", id_field, field1>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("extend message with new field constraint", "[message]") {
    using base_defn = message<"msg_base", id_field, field1>;
    using defn = extend<base_defn, "msg", field1::with_required<1>>;
    using expected_defn = message<"msg", id_field, field1::with_required<1>>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("message equivalence (owning)", "[message]") {
    test_msg m1{"f1"_field = 0xba11, "f2"_field = 0x42, "f3"_field = 0xd00d};
    test_msg m2{"f1"_field = 0xba11, "f2"_field = 0x42, "f3"_field = 0xd00d};
    CHECK(equivalent(m1, m2));
    test_msg other{"f1"_field = 0xba11, "f2"_field = 0x42, "f3"_field = 0xd00f};
    CHECK(not equivalent(m1, other));
}

TEST_CASE("message equivalence (owning/const_view)", "[message]") {
    owning<msg_defn> m{"f1"_field = 0xba11, "f2"_field = 0x42,
                       "f3"_field = 0xd00d};
    auto cv = m.as_const_view();
    CHECK(equivalent(m, cv));
    CHECK(equivalent(cv, m));

    owning<msg_defn> other{"f1"_field = 0xba11, "f2"_field = 0x42,
                           "f3"_field = 0xd00f};
    CHECK(not equivalent(other, cv));
    CHECK(not equivalent(cv, other));
}

TEST_CASE("message equivalence (owning/mutable_view)", "[message]") {
    owning<msg_defn> m{"f1"_field = 0xba11, "f2"_field = 0x42,
                       "f3"_field = 0xd00d};
    auto mv = m.as_mutable_view();
    CHECK(equivalent(m, mv));
    CHECK(equivalent(mv, m));

    owning<msg_defn> other{"f1"_field = 0xba11, "f2"_field = 0x42,
                           "f3"_field = 0xd00f};
    CHECK(not equivalent(other, mv));
    CHECK(not equivalent(mv, other));
}

TEST_CASE("message equivalence (views)", "[message]") {
    owning<msg_defn> m{"f1"_field = 0xba11, "f2"_field = 0x42,
                       "f3"_field = 0xd00d};
    auto cv1 = m.as_const_view();
    auto mv1 = m.as_mutable_view();
    CHECK(equivalent(cv1, cv1));
    CHECK(equivalent(mv1, mv1));
    CHECK(equivalent(cv1, mv1));
    CHECK(equivalent(mv1, cv1));

    owning<msg_defn> other{"f1"_field = 0xba11, "f2"_field = 0x42,
                           "f3"_field = 0xd00f};
    auto cv2 = other.as_const_view();
    CHECK(not equivalent(cv1, cv2));
}

namespace {
template <typename View>
struct MsgEquivMatcher : Catch::Matchers::MatcherGenericBase {
    MsgEquivMatcher(View v) : view{v} {}

    template <typename OtherMsg> bool match(OtherMsg const &other) const {
        return msg::equivalent(view, other);
    }

    std::string describe() const override {
        return "Equivalent: " + Catch::rangeToString(view.data());
    }

  private:
    View view;
};

template <typename Msg>
auto MsgEquiv(Msg const &msg) -> MsgEquivMatcher<typename Msg::const_view_t> {
    return {msg.as_const_view()};
}
} // namespace

TEST_CASE("message equivalence matcher", "[message]") {
    owning<msg_defn> m{"f1"_field = 0xba11, "f2"_field = 0x42,
                       "f3"_field = 0xd00d};
    auto cv1 = m.as_const_view();
    auto mv1 = m.as_mutable_view();

    CHECK_THAT(cv1, MsgEquiv(m));
    CHECK_THAT(mv1, MsgEquiv(m));

    owning<msg_defn> m2{"f1"_field = 0xba12, "f2"_field = 0x42,
                        "f3"_field = 0xd00d};
    CHECK_THAT(m2, not MsgEquiv(m));
}

TEST_CASE("message reports size", "[message]") {
    STATIC_REQUIRE(msg_defn::size<std::uint32_t>::value == 2);
    STATIC_REQUIRE(msg_defn::size<std::uint16_t>::value == 4);
    STATIC_REQUIRE(msg_defn::size<std::uint8_t>::value == 7);
}

TEST_CASE("shift a field by bit offset", "[message]") {
    using new_field = id_field::shifted_by<3>;
    using expected_field =
        field<"id", std::uint32_t>::located<at{34_msb, 27_lsb}>;
    STATIC_REQUIRE(std::is_same_v<new_field, expected_field>);
}

TEST_CASE("shift a field by byte offset", "[message]") {
    using new_field = id_field::shifted_by<1, std::uint8_t>;
    using expected_field =
        field<"id", std::uint32_t>::located<at{39_msb, 32_lsb}>;
    STATIC_REQUIRE(std::is_same_v<new_field, expected_field>);
}

TEST_CASE("shift all fields in a message", "[message]") {
    using base_defn = message<"msg", id_field, field1>;
    using defn = base_defn::shifted_by<1, std::uint8_t>;

    using shifted_id = id_field::shifted_by<1, std::uint8_t>;
    using shifted_field1 = field1::shifted_by<1, std::uint8_t>;
    using expected_defn = message<"msg", shifted_id, shifted_field1>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("combine messages", "[message]") {
    using f1 = field<"f1", std::uint32_t>::located<at{23_msb, 16_lsb}>;
    using f2 = field<"f2", std::uint32_t>::located<at{15_msb, 0_lsb}>;
    using m1 = message<"m1", f1, f2>;

    using f3 = field<"f3", std::uint32_t>::located<at{23_msb, 16_lsb}>;
    using f4 = field<"f4", std::uint32_t>::located<at{15_msb, 0_lsb}>;
    using m2 = message<"m2", f3, f4>;

    using defn = combine<"defn", m1, m2::shifted_by<1, std::uint32_t>>;
    using expected_defn =
        message<"defn", f1, f2, f3::shifted_by<1, std::uint32_t>,
                f4::shifted_by<1, std::uint32_t>>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("combine 1 message", "[message]") {
    using f1 = field<"f1", std::uint32_t>::located<at{15_msb, 0_lsb}>;
    using f2 = field<"f2", std::uint32_t>::located<at{23_msb, 16_lsb}>;
    using m1 = message<"m1", f1, f2>;

    using defn = combine<"defn", m1>;
    using expected_defn = message<"defn", f1, f2>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("pack messages", "[message]") {
    using f1 = field<"f1", std::uint32_t>::located<at{15_msb, 0_lsb}>;
    using f2 = field<"f2", std::uint32_t>::located<at{23_msb, 16_lsb}>;
    using m1 = message<"m1", f1, f2>;

    using f3 = field<"f3", std::uint32_t>::located<at{15_msb, 0_lsb}>;
    using f4 = field<"f4", std::uint32_t>::located<at{23_msb, 16_lsb}>;
    using m2 = message<"m2", f3, f4>;

    using defn = pack<"defn", std::uint8_t, m1, m2>;
    using expected_defn =
        message<"defn", f1, f2,
                f3::shifted_by<m1::size<std::uint8_t>::value, std::uint8_t>,
                f4::shifted_by<m1::size<std::uint8_t>::value, std::uint8_t>>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("pack 1 message", "[message]") {
    using f1 = field<"f1", std::uint32_t>::located<at{15_msb, 0_lsb}>;
    using f2 = field<"f2", std::uint32_t>::located<at{23_msb, 16_lsb}>;
    using m1 = message<"m1", f1, f2>;

    using defn = pack<"defn", std::uint8_t, m1>;
    using expected_defn = message<"defn", f1, f2>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("pack with empty messages", "[message]") {
    using m0 = message<"m0">;

    using f1 = field<"f1", std::uint32_t>::located<at{15_msb, 0_lsb}>;
    using f2 = field<"f2", std::uint32_t>::located<at{23_msb, 16_lsb}>;
    using m1 = message<"m1", f1, f2>;

    using m2 = message<"m2">;

    using f3 = field<"f3", std::uint32_t>::located<at{15_msb, 0_lsb}>;
    using f4 = field<"f4", std::uint32_t>::located<at{23_msb, 16_lsb}>;
    using m3 = message<"m3", f3, f4>;

    using defn = pack<"defn", std::uint8_t, m0, m1, m2, m3>;
    using expected_defn =
        message<"defn", f1, f2,
                f3::shifted_by<m1::size<std::uint8_t>::value, std::uint8_t>,
                f4::shifted_by<m1::size<std::uint8_t>::value, std::uint8_t>>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

namespace {
[[maybe_unused]] constexpr inline struct custom_t {
    template <typename T>
        requires true // more constrained
    CONSTEVAL auto operator()(T &&t) const
        noexcept(noexcept(std::forward<T>(t).query(std::declval<custom_t>())))
            -> decltype(std::forward<T>(t).query(*this)) {
        return std::forward<T>(t).query(*this);
    }

    CONSTEVAL auto operator()(auto &&) const { return 42; }
} custom;
} // namespace

TEST_CASE("message with default empty environment", "[message]") {
    STATIC_REQUIRE(custom(msg_defn::env_t{}) == 42);
}

TEST_CASE("message with defined environment", "[message]") {
    using env_t = stdx::make_env_t<custom, 17>;
    using defn = message<"msg", env_t, id_field::with_required<0x80>, field1,
                         field2, field3>;
    STATIC_REQUIRE(custom(defn::env_t{}) == 17);
}

TEST_CASE("supplement message environment", "[message]") {
    using env_t = stdx::make_env_t<custom, 17>;
    using defn = message<"msg", env_t, id_field::with_required<0x80>, field1,
                         field2, field3>;
    using new_defn = defn::with_env<stdx::make_env_t<custom, 18>>;
    STATIC_REQUIRE(custom(new_defn::env_t{}) == 18);
}

TEST_CASE("combine appends environments", "[message]") {
    using env1_t = stdx::make_env_t<custom, 17>;
    using m1 = message<"m1", env1_t>;

    using env2_t = stdx::make_env_t<custom, 18>;
    using m2 = message<"m2", env2_t>;

    using defn = combine<"defn", m1, m2>;
    STATIC_REQUIRE(custom(defn::env_t{}) == 18);
}

TEST_CASE("pack appends environments", "[message]") {
    using env1_t = stdx::make_env_t<custom, 17>;
    using m1 = message<"m1", env1_t>;

    using env2_t = stdx::make_env_t<custom, 18>;
    using m2 = message<"m2", env2_t>;

    using defn = pack<"defn", std::uint8_t, m1, m2>;
    STATIC_REQUIRE(custom(defn::env_t{}) == 18);
}
