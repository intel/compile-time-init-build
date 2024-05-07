#include <log/fmt/logger.hpp>
#include <msg/message.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using namespace msg;

using id_field = field<"id", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;
using field1 = field<"f1", std::uint32_t>::located<at{0_dw, 15_msb, 0_lsb}>;
using field2 = field<"f2", std::uint32_t>::located<at{1_dw, 23_msb, 16_lsb}>;
using field3 = field<"f3", std::uint32_t>::located<at{1_dw, 15_msb, 8_lsb},
                                                   at{1_dw, 7_msb, 0_lsb}>;

using msg_defn =
    message<"msg", id_field::WithRequired<0x80>, field1, field2, field3>;
using test_msg = owning<msg_defn>;

std::string log_buffer{};
} // namespace

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(log_buffer)};

TEST_CASE("message with automatic storage", "[message]") {
    test_msg msg{};
    auto data = msg.data();
    static_assert(std::is_same_v<decltype(data), stdx::span<std::uint32_t, 2>>);
}

TEST_CASE("message with custom storage", "[message]") {
    auto arr = std::array<std::uint16_t, 4>{0x0, 0x8000, 0xd00d, 0x0042};
    msg_defn::owner_t msg{arr, "f1"_field = 0xba11};
    CHECK(0x80 == msg.get("id"_field));
    CHECK(0xba11 == msg.get("f1"_field));
    static_assert(
        std::is_same_v<typename decltype(msg)::storage_t, decltype(arr)>);
}

TEST_CASE("message constructed from span", "[message]") {
    auto const arr =
        typename msg_defn::default_storage_t{0x8000'ba11, 0x0042'd00d};
    auto s = stdx::span{arr};

    msg_defn::owner_t msg{s};
    static_assert(std::is_same_v<typename decltype(msg)::storage_t,
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
    mutable_view<msg_defn> msg{arr, "id"_field = 0x81};

    msg.set("f1"_field = 0);
    CHECK(0x8100'0000 == arr[0]);
}

TEST_CASE("view constructed from span (mutable)", "[message]") {
    auto arr = typename msg_defn::default_storage_t{0x8000'ba11, 0x0042'd00d};
    auto s = stdx::span{arr};
    msg_defn::view_t msg{s, "id"_field = 0x81};

    msg.set("f1"_field = 0);
    CHECK(0x8100'0000 == arr[0]);
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
    msg_defn::view_t view{msg, "id"_field = 0x81};
    CHECK(0x81 == msg.get("id"_field));

    view.set("f1"_field = 0xba12);
    CHECK(0xba12 == msg.get("f1"_field));
}

TEST_CASE("message constructed from view", "[message]") {
    auto const arr =
        typename msg_defn::default_storage_t{0x8000'ba11, 0x0042'd00d};
    msg::const_view<msg_defn> v{arr};

    msg_defn::owner_t msg{v};
    static_assert(std::is_same_v<typename decltype(msg)::storage_t,
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
    static_assert(
        std::is_same_v<decltype(v.data()),
                       typename test_msg::definition_t::default_const_span_t>);
    msg.set("f1"_field = 0xba11);
    CHECK(0xba11 == v.get("f1"_field));
}

TEST_CASE("mutable view from owning message", "[message]") {
    test_msg msg{};
    auto v = msg.as_mutable_view();
    static_assert(
        std::is_same_v<decltype(v.data()),
                       typename test_msg::definition_t::default_span_t>);
    v.set("f1"_field = 0xba11);
    CHECK(0xba11 == msg.get("f1"_field));
}

TEST_CASE("equal_to matcher", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(id_field::equal_to<0x80>(msg));
    CHECK(field1::equal_to<0xba11>(msg));
    CHECK(field2::equal_to<0x42>(msg));
    CHECK(field3::equal_to<0xd00d>(msg));

    CHECK(not id_field::equal_to<0x0>(msg));
    CHECK(not field1::equal_to<0x0>(msg));
    CHECK(not field2::equal_to<0x0>(msg));
    CHECK(not field3::equal_to<0x0>(msg));
}

TEST_CASE("default matcher", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x0, 0x00}};

    CHECK(id_field::match_default(msg));
    CHECK(field1::match_default(msg));
    CHECK(field2::match_default(msg));
    CHECK(field3::match_default(msg));
}

TEST_CASE("default matcher (no match)", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(not id_field::match_default(msg));
    CHECK(not field1::match_default(msg));
    CHECK(not field2::match_default(msg));
    CHECK(not field3::match_default(msg));
}

TEST_CASE("in matcher", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK((id_field::in<0x80>(msg)));
    CHECK((field1::in<0xba11>(msg)));
    CHECK((field2::in<0x42>(msg)));
    CHECK((field3::in<0xd00d>(msg)));

    CHECK(not id_field::in<0x11>(msg));
    CHECK(not field1::in<0x1111>(msg));
    CHECK(not field2::in<0x11>(msg));
    CHECK(not field3::in<0x1111>(msg));

    CHECK(id_field::in<0x80, 0x0>(msg));
    CHECK(field1::in<0xba11, 0x0>(msg));
    CHECK(field2::in<0x42, 0x0>(msg));
    CHECK(field3::in<0xd00d, 0x0>(msg));

    CHECK(not id_field::in<0x11, 0x0>(msg));
    CHECK(not field1::in<0x1111, 0x0>(msg));
    CHECK(not field2::in<0x11, 0x0>(msg));
    CHECK(not field3::in<0x1111, 0x0>(msg));

    CHECK(id_field::in<0x80, 0x0, 0xE>(msg));
    CHECK(field1::in<0xba11, 0x0, 0xEEEE>(msg));
    CHECK(field2::in<0x42, 0x0, 0xEE>(msg));
    CHECK(field3::in<0xd00d, 0x0, 0xEEEE>(msg));

    CHECK(not id_field::in<0x11, 0x0, 0xE>(msg));
    CHECK(not field1::in<0x1111, 0x0, 0xEEEE>(msg));
    CHECK(not field2::in<0x11, 0x0, 0xEE>(msg));
    CHECK(not field3::in<0x1111, 0x0, 0xEEEE>(msg));
}

TEST_CASE("greater_than matcher", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(not id_field::greater_than<0x80>(msg));
    CHECK(not field1::greater_than<0xba11>(msg));
    CHECK(not field2::greater_than<0x42>(msg));
    CHECK(not field3::greater_than<0xd00d>(msg));

    CHECK(id_field::greater_than<0x11>(msg));
    CHECK(field1::greater_than<0x1111>(msg));
    CHECK(field2::greater_than<0x11>(msg));
    CHECK(field3::greater_than<0x1111>(msg));
}

TEST_CASE("greater_than_or_equal_to matcher", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(not id_field::greater_than_or_equal_to<0xEE>(msg));
    CHECK(not field1::greater_than_or_equal_to<0xEEEE>(msg));
    CHECK(not field2::greater_than_or_equal_to<0xEE>(msg));
    CHECK(not field3::greater_than_or_equal_to<0xEEEE>(msg));

    CHECK(id_field::greater_than_or_equal_to<0x80>(msg));
    CHECK(field1::greater_than_or_equal_to<0xba11>(msg));
    CHECK(field2::greater_than_or_equal_to<0x42>(msg));
    CHECK(field3::greater_than_or_equal_to<0xd00d>(msg));

    CHECK(id_field::greater_than_or_equal_to<0x11>(msg));
    CHECK(field1::greater_than_or_equal_to<0x1111>(msg));
    CHECK(field2::greater_than_or_equal_to<0x11>(msg));
    CHECK(field3::greater_than_or_equal_to<0x1111>(msg));
}

TEST_CASE("less_than matcher", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(not id_field::less_than<0x80>(msg));
    CHECK(not field1::less_than<0xba11>(msg));
    CHECK(not field2::less_than<0x42>(msg));
    CHECK(not field3::less_than<0xd00d>(msg));

    CHECK(id_field::less_than<0xEE>(msg));
    CHECK(field1::less_than<0xEEEE>(msg));
    CHECK(field2::less_than<0xEE>(msg));
    CHECK(field3::less_than<0xEEEE>(msg));
}

TEST_CASE("less_than_or_equal_to matcher", "[message]") {
    test_msg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(id_field::less_than_or_equal_to<0xEE>(msg));
    CHECK(field1::less_than_or_equal_to<0xEEEE>(msg));
    CHECK(field2::less_than_or_equal_to<0xEE>(msg));
    CHECK(field3::less_than_or_equal_to<0xEEEE>(msg));

    CHECK(id_field::less_than_or_equal_to<0x80>(msg));
    CHECK(field1::less_than_or_equal_to<0xba11>(msg));
    CHECK(field2::less_than_or_equal_to<0x42>(msg));
    CHECK(field3::less_than_or_equal_to<0xd00d>(msg));

    CHECK(not id_field::less_than_or_equal_to<0x11>(msg));
    CHECK(not field1::less_than_or_equal_to<0x1111>(msg));
    CHECK(not field2::less_than_or_equal_to<0x11>(msg));
    CHECK(not field3::less_than_or_equal_to<0x1111>(msg));
}

TEST_CASE("describe a message", "[message]") {
    test_msg msg{"f1"_field = 0xba11, "f2"_field = 0x42, "f3"_field = 0xd00d};
    CIB_INFO("{}", msg.describe());
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
    static_assert(std::is_same_v<decltype(msg.data()),
                                 stdx::span<std::uint32_t const, 4>>);
    const_view<msg_defn> v = msg;
    static_assert(
        std::is_same_v<decltype(v.data()), stdx::span<std::uint32_t const, 2>>);
    CHECK(msg.data()[0] == v.data()[0]);
    CHECK(msg.data()[1] == v.data()[1]);
}

TEST_CASE("view_of concept", "[message]") {
    auto const arr = std::array<std::uint32_t, 4>{0x8000'ba11, 0x0042'd00d};
    msg_defn::view_t msg{arr};
    static_assert(msg::view_of<decltype(msg), msg_defn>);

    const_view<msg_defn> v = msg;
    static_assert(msg::view_of<decltype(v), msg_defn>);
    static_assert(msg::const_view_of<decltype(v), msg_defn>);
}

TEST_CASE("message fields are canonically sorted by lsb", "[message]") {
    using defn = message<"msg", field2, field1>;
    static_assert(std::is_same_v<defn, message<"msg", field1, field2>>);
    static_assert(std::is_same_v<defn, detail::message<"msg", field1, field2>>);
}

TEST_CASE("extend message with more fields", "[message]") {
    using base_defn = message<"msg_base", id_field, field1>;
    using defn = extend<base_defn, "msg", field2, field3>;
    using expected_defn = message<"msg", id_field, field1, field2, field3>;
    static_assert(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("extend message with duplicate field", "[message]") {
    using base_defn = message<"msg_base", id_field, field1>;
    using defn = extend<base_defn, "msg", field1>;
    using expected_defn = message<"msg", id_field, field1>;
    static_assert(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("extend message with new field constraint", "[message]") {
    using base_defn = message<"msg_base", id_field, field1>;
    using defn = extend<base_defn, "msg", field1::WithRequired<1>>;
    using expected_defn = message<"msg", id_field, field1::WithRequired<1>>;
    static_assert(std::is_same_v<defn, expected_defn>);
}
