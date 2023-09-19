#include <cib/cib.hpp>
#include <msg/field.hpp>
#include <msg/indexed_callback.hpp>
#include <msg/indexed_service.hpp>
#include <msg/match.hpp>
#include <msg/message.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using test_id_field =
    msg::field<decltype("test_id_field"_sc), 0, 31, 24, std::uint32_t>;
using test_opcode_field =
    msg::field<decltype("test_opcode_field"_sc), 0, 15, 0, std::uint32_t>;
using test_field_2 =
    msg::field<decltype("test_field_2"_sc), 1, 23, 16, std::uint32_t>;
using test_field_3 =
    msg::field<decltype("test_field_3"_sc), 1, 15, 0, std::uint32_t>;

using test_msg_t =
    msg::message_base<decltype("test_msg"_sc), 2, test_id_field,
                      test_opcode_field, test_field_2, test_field_3>;

using index_spec = decltype(stdx::make_indexed_tuple<msg::get_field_type>(
    msg::temp_index<test_id_field, 256, 32>{},
    msg::temp_index<test_opcode_field, 256, 32>{}));

struct test_service : msg::indexed_service<index_spec, test_msg_t> {};

bool callback_success;

constexpr auto test_callback = msg::indexed_callback_t(
    "TestCallback"_sc, match::all(test_id_field::in<0x80>),
    [](test_msg_t const &) { callback_success = true; });

struct test_project {
    constexpr static auto config = cib::config(
        cib::exports<test_service>, cib::extend<test_service>(test_callback));
};
} // namespace

TEST_CASE("build handler", "[indexed_builder]") {
    cib::nexus<test_project> test_nexus{};
    test_nexus.init();

    callback_success = false;
    cib::service<test_service>->handle(test_msg_t{test_id_field{0x80}});
    CHECK(callback_success);

    callback_success = false;
    cib::service<test_service>->handle(test_msg_t{test_id_field{0x81}});
    CHECK(not callback_success);
}

namespace {
constexpr auto test_callback_equals = msg::indexed_callback_t(
    "TestCallback"_sc, test_id_field::equal_to<0x80>,
    [](test_msg_t const &) { callback_success = true; });

struct test_project_equals {
    constexpr static auto config =
        cib::config(cib::exports<test_service>,
                    cib::extend<test_service>(test_callback_equals));
};
} // namespace

TEST_CASE("build handler field equal_to", "[indexed_builder]") {
    cib::nexus<test_project_equals> test_nexus{};
    test_nexus.init();

    callback_success = false;
    cib::service<test_service>->handle(test_msg_t{test_id_field{0x80}});
    CHECK(callback_success);

    callback_success = false;
    cib::service<test_service>->handle(test_msg_t{test_id_field{0x81}});
    CHECK(not callback_success);
}

namespace {
constexpr auto test_callback_multi_field = msg::indexed_callback_t(
    "test_callback_multi_field"_sc,
    match::all(test_id_field::in<0x80, 0x42>, test_opcode_field::equal_to<1>),
    [](test_msg_t const &) { callback_success = true; });

bool callback_success_single_field;

constexpr auto test_callback_single_field = msg::indexed_callback_t(
    "test_callback_single_field"_sc, match::all(test_id_field::equal_to<0x50>),
    [](test_msg_t const &) { callback_success_single_field = true; });

struct test_project_multi_field {
    constexpr static auto config =
        cib::config(cib::exports<test_service>,
                    cib::extend<test_service>(test_callback_multi_field,
                                              test_callback_single_field));
};
} // namespace

TEST_CASE("build handler multi fields", "[indexed_builder]") {
    cib::nexus<test_project_multi_field> test_nexus{};
    test_nexus.init();

    callback_success = false;
    callback_success_single_field = false;
    cib::service<test_service>->handle(
        test_msg_t{test_id_field{0x80}, test_opcode_field{1}});
    CHECK(callback_success);
    CHECK(not callback_success_single_field);

    callback_success = false;
    callback_success_single_field = false;
    cib::service<test_service>->handle(test_msg_t{test_id_field{0x81}});
    CHECK(not callback_success);
    CHECK(not callback_success_single_field);

    // make sure an unconstrained field in a callback doesn't cause a mismatch
    callback_success = false;
    callback_success_single_field = false;
    cib::service<test_service>->handle(
        test_msg_t{test_id_field{0x50}, test_opcode_field{1}});
    CHECK(not callback_success);
    CHECK(callback_success_single_field);
}

namespace {
using partial_index_spec =
    decltype(stdx::make_indexed_tuple<msg::get_field_type>(
        msg::temp_index<test_id_field, 256, 32>{}));

struct partially_indexed_test_service
    : msg::indexed_service<partial_index_spec, test_msg_t> {};

struct partially_indexed_test_project {
    constexpr static auto config = cib::config(
        cib::exports<partially_indexed_test_service>,
        cib::extend<partially_indexed_test_service>(test_callback_multi_field));
};
} // namespace

TEST_CASE("message matching partial index but not callback matcher",
          "[indexed_builder]") {
    cib::nexus<partially_indexed_test_project> test_nexus{};
    test_nexus.init();

    callback_success = false;
    cib::service<partially_indexed_test_service>->handle(
        test_msg_t{test_id_field{0x80}, test_opcode_field{2}});
    CHECK(not callback_success);
}
