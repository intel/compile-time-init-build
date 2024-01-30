#include <cib/cib.hpp>
#include <log/fmt/logger.hpp>
#include <match/ops.hpp>
#include <msg/callback.hpp>
#include <msg/field.hpp>
#include <msg/message.hpp>
#include <msg/rle_indexed_service.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <iterator>
#include <string>

namespace {
using namespace msg;

using test_id_field =
    field<"test_id_field", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;
using test_opcode_field =
    field<"test_opcode_field", std::uint32_t>::located<at{0_dw, 15_msb, 0_lsb}>;
using test_field_2 =
    field<"test_field_2", std::uint32_t>::located<at{1_dw, 23_msb, 16_lsb}>;
using test_field_3 =
    field<"test_field_3", std::uint32_t>::located<at{1_dw, 15_msb, 0_lsb}>;

using msg_defn = message<"test_msg", test_id_field, test_opcode_field,
                         test_field_2, test_field_3>;
using test_msg_t = owning<msg_defn>;

using index_spec = index_spec<test_id_field, test_opcode_field>;
struct test_service : rle_indexed_service<index_spec, test_msg_t> {};

bool callback_success;

constexpr auto test_callback = msg::callback<"TestCallback", msg_defn>(
    test_id_field::in<0x80>,
    [](test_msg_t const &) { callback_success = true; });

struct test_project {
    constexpr static auto config = cib::config(
        cib::exports<test_service>, cib::extend<test_service>(test_callback));
};

std::string log_buffer{};
} // namespace

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(log_buffer)};

TEST_CASE("build rle handler", "[rle_indexed_builder]") {
    cib::nexus<test_project> test_nexus{};
    test_nexus.init();

    callback_success = false;
    cib::service<test_service>->handle(
        test_msg_t{"test_id_field"_field = 0x80});
    CHECK(callback_success);

    callback_success = false;
    cib::service<test_service>->handle(
        test_msg_t{"test_id_field"_field = 0x81});
    CHECK(not callback_success);
}

TEST_CASE("match rle output success", "[rle_handler_builder]") {
    log_buffer.clear();
    cib::nexus<test_project> test_nexus{};
    test_nexus.init();

    cib::service<test_service>->handle(
        test_msg_t{"test_id_field"_field = 0x80});
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("Incoming message matched") != std::string::npos);
    CHECK(log_buffer.find("[TestCallback]") != std::string::npos);
    CHECK(log_buffer.find("[test_id_field == 0x80]") != std::string::npos);
}

TEST_CASE("match rle output failure", "[rle_handler_builder]") {
    log_buffer.clear();
    cib::nexus<test_project> test_nexus{};
    test_nexus.init();

    cib::service<test_service>->handle(
        test_msg_t{"test_id_field"_field = 0x81});
    CHECK(log_buffer.find(
              "None of the registered callbacks claimed this message") !=
          std::string::npos);
}

namespace {
constexpr auto test_callback_equals = msg::callback<"TestCallback", msg_defn>(
    test_id_field::equal_to<0x80>,
    [](test_msg_t const &) { callback_success = true; });

struct test_project_equals {
    constexpr static auto config =
        cib::config(cib::exports<test_service>,
                    cib::extend<test_service>(test_callback_equals));
};
} // namespace

TEST_CASE("build rle handler field equal_to", "[rle_indexed_builder]") {
    cib::nexus<test_project_equals> test_nexus{};
    test_nexus.init();

    callback_success = false;
    cib::service<test_service>->handle(
        test_msg_t{"test_id_field"_field = 0x80});
    CHECK(callback_success);

    callback_success = false;
    cib::service<test_service>->handle(
        test_msg_t{"test_id_field"_field = 0x81});
    CHECK(not callback_success);
}

namespace {
constexpr auto test_callback_multi_field =
    msg::callback<"test_callback_multi_field", msg_defn>(
        test_id_field::in<0x80, 0x42> and test_opcode_field::equal_to<1>,
        [](test_msg_t const &) { callback_success = true; });

bool callback_success_single_field;

constexpr auto test_callback_single_field =
    msg::callback<"test_callback_single_field", msg_defn>(
        test_id_field::equal_to<0x50>,
        [](test_msg_t const &) { callback_success_single_field = true; });

struct test_project_multi_field {
    constexpr static auto config =
        cib::config(cib::exports<test_service>,
                    cib::extend<test_service>(test_callback_multi_field,
                                              test_callback_single_field));
};
} // namespace

TEST_CASE("build rle handler multi fields", "[rle_indexed_builder]") {
    cib::nexus<test_project_multi_field> test_nexus{};
    test_nexus.init();

    callback_success = false;
    callback_success_single_field = false;
    cib::service<test_service>->handle(test_msg_t{
        "test_id_field"_field = 0x80, "test_opcode_field"_field = 1});
    CHECK(callback_success);
    CHECK(not callback_success_single_field);

    callback_success = false;
    callback_success_single_field = false;
    cib::service<test_service>->handle(
        test_msg_t{"test_id_field"_field = 0x81});
    CHECK(not callback_success);
    CHECK(not callback_success_single_field);

    // an unconstrained field in a callback doesn't cause a mismatch
    callback_success = false;
    callback_success_single_field = false;
    cib::service<test_service>->handle(test_msg_t{
        "test_id_field"_field = 0x50, "test_opcode_field"_field = 1});
    CHECK(not callback_success);
    CHECK(callback_success_single_field);
}

namespace {
using partial_index_spec = msg::index_spec<test_id_field>;

struct partially_indexed_test_service
    : msg::rle_indexed_service<partial_index_spec, test_msg_t> {};

struct partially_indexed_test_project {
    constexpr static auto config = cib::config(
        cib::exports<partially_indexed_test_service>,
        cib::extend<partially_indexed_test_service>(test_callback_multi_field));
};
} // namespace

TEST_CASE("message matching partial rle index but not callback matcher",
          "[rle_indexed_builder]") {
    cib::nexus<partially_indexed_test_project> test_nexus{};
    test_nexus.init();

    callback_success = false;
    cib::service<partially_indexed_test_service>->handle(test_msg_t{
        "test_id_field"_field = 0x80, "test_opcode_field"_field = 2});
    CHECK(not callback_success);
}
