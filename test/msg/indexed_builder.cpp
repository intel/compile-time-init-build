#include <cib/cib.hpp>
#include <msg/indexed_callback.hpp>
#include <msg/indexed_service.hpp>

#include <catch2/catch_test_macros.hpp>

namespace msg {

using test_id_field =
    field<decltype("test_id_field"_sc), 0, 31, 24, std::uint32_t>;
using test_opcode_field =
    field<decltype("test_opcode_field"_sc), 0, 15, 0, std::uint32_t>;
using test_field_2 =
    field<decltype("test_field_2"_sc), 1, 23, 16, std::uint32_t>;
using test_field_3 =
    field<decltype("test_field_3"_sc), 1, 15, 0, std::uint32_t>;

using test_msg_t = message_base<decltype("test_msg"_sc), 2, test_id_field,
                                test_opcode_field, test_field_2, test_field_3>;

using index_spec = decltype(cib::make_indexed_tuple<get_field_type>(
    temp_index<test_id_field, 256, 32>{},
    temp_index<test_opcode_field, 256, 32>{}));

struct test_service : ::msg::indexed_service<index_spec, test_msg_t> {};

static inline bool callback_success;

[[maybe_unused]] constexpr static auto test_callback = msg::indexed_callback_t(
    "TestCallback"_sc, match::all(test_id_field::in<0x80>),
    [](test_msg_t const &) { callback_success = true; });

struct test_project {
    [[maybe_unused]] constexpr static auto config = cib::config(
        cib::exports<test_service>, cib::extend<test_service>(test_callback));
};

TEST_CASE("build handler", "[handler_builder]") {
    [[maybe_unused]] cib::nexus<test_project> test_nexus{};
    test_nexus.init();

    // callback_success = false;
    // cib::service<test_service>->handle(test_msg_t{test_id_field{0x80}});
    // REQUIRE(callback_success);

    // callback_success = false;
    // cib::service<test_service>->handle(test_msg_t{test_id_field{0x81}});
    // REQUIRE_FALSE(callback_success);
}

// constexpr static auto test_callback_equals = msg::indexed_callback_t(
//     "TestCallback"_sc, test_id_field::equal_to<0x80>,
//     [](test_msg_t const &) { callback_success = true; });

// struct test_project_equals {
//     constexpr static auto config =
//         cib::config(cib::exports<test_service>,
//                     cib::extend<test_service>(test_callback_equals));
// };

// TEST_CASE("build handler field equal_to", "[handler_builder]") {
//     cib::nexus<test_project_equals> test_nexus{};
//     test_nexus.init();

//     callback_success = false;
//     cib::service<test_service>->handle(test_msg_t{test_id_field{0x80}});
//     REQUIRE(callback_success);

//     callback_success = false;
//     cib::service<test_service>->handle(test_msg_t{test_id_field{0x81}});
//     REQUIRE_FALSE(callback_success);
// }

// constexpr static auto test_callback_multi_field = msg::indexed_callback_t(
//     "test_callback_multi_field"_sc,

//     match::all(test_id_field::in<0x80, 0x42>,
//     test_opcode_field::equal_to<1>),

//     [](test_msg_t const &) { callback_success = true; });

// static inline bool callback_success_single_field;

// constexpr static auto test_callback_single_field = msg::indexed_callback_t(
//     "test_callback_single_field"_sc,

//     match::all(test_id_field::equal_to<0x50>),

//     [](test_msg_t const &) { callback_success_single_field = true; });

// struct test_project_multi_field {
//     constexpr static auto config =
//         cib::config(cib::exports<test_service>,
//                     cib::extend<test_service>(test_callback_multi_field,
//                                               test_callback_single_field));
// };

// TEST_CASE("build handler multi fields", "[handler_builder]") {
//     cib::nexus<test_project_multi_field> test_nexus{};
//     test_nexus.init();

//     callback_success = false;
//     callback_success_single_field = false;
//     cib::service<test_service>->handle(
//         test_msg_t{test_id_field{0x80}, test_opcode_field{1}});
//     REQUIRE(callback_success);
//     REQUIRE_FALSE(callback_success_single_field);

//     callback_success = false;
//     callback_success_single_field = false;
//     cib::service<test_service>->handle(test_msg_t{test_id_field{0x81}});
//     REQUIRE_FALSE(callback_success);
//     REQUIRE_FALSE(callback_success_single_field);

//     // make sure an unconstrained field in a callback doesn't cause a
//     mismatch callback_success = false; callback_success_single_field = false;
//     cib::service<test_service>->handle(
//         test_msg_t{test_id_field{0x50}, test_opcode_field{1}});
//     REQUIRE_FALSE(callback_success);
//     REQUIRE(callback_success_single_field);
// }
} // namespace msg
