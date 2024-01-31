#include <cib/cib.hpp>
#include <log/fmt/logger.hpp>
#include <match/ops.hpp>
#include <msg/callback.hpp>
#include <msg/field.hpp>
#include <msg/indexed_service.hpp>
#include <msg/message.hpp>

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
struct test_service : indexed_service<index_spec, test_msg_t> {};

bool callback_success;

constexpr auto test_callback = callback<"TestCallback", msg_defn>(
    test_id_field::in<0x80>, [](auto) { callback_success = true; });

struct test_project {
    constexpr static auto config = cib::config(
        cib::exports<test_service>, cib::extend<test_service>(test_callback));
};

std::string log_buffer{};
} // namespace

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(log_buffer)};

TEST_CASE("build handler", "[indexed_builder]") {
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

TEST_CASE("match output success", "[handler_builder]") {
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

TEST_CASE("match output failure", "[handler_builder]") {
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
    test_id_field::equal_to<0x80>, [](auto) { callback_success = true; });

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
        [](auto) { callback_success = true; });

bool callback_success_single_field;

constexpr auto test_callback_single_field =
    msg::callback<"test_callback_single_field", msg_defn>(
        test_id_field::equal_to<0x50>,
        [](auto) { callback_success_single_field = true; });

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

    log_buffer.clear();
    callback_success = false;
    callback_success_single_field = false;
    cib::service<test_service>->handle(test_msg_t{
        "test_id_field"_field = 0x80, "test_opcode_field"_field = 1});
    CHECK(callback_success);
    CHECK(not callback_success_single_field);
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("(collapsed to [true])") != std::string::npos);

    callback_success = false;
    callback_success_single_field = false;
    cib::service<test_service>->handle(
        test_msg_t{"test_id_field"_field = 0x81});
    CHECK(not callback_success);
    CHECK(not callback_success_single_field);

    // an unconstrained field in a callback doesn't cause a mismatch
    log_buffer.clear();
    callback_success = false;
    callback_success_single_field = false;
    cib::service<test_service>->handle(test_msg_t{
        "test_id_field"_field = 0x50, "test_opcode_field"_field = 1});
    CHECK(not callback_success);
    CHECK(callback_success_single_field);
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("(collapsed to [true])") != std::string::npos);
}

namespace {
using partial_index_spec = msg::index_spec<test_id_field>;

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

    log_buffer.clear();
    callback_success = false;
    cib::service<partially_indexed_test_service>->handle(test_msg_t{
        "test_id_field"_field = 0x80, "test_opcode_field"_field = 2});
    CHECK(not callback_success);
}

namespace {
constexpr auto test_callback_not_single_field =
    msg::callback<"test_callback_not_single_field", msg_defn>(
        not test_id_field::equal_to<0x50>,
        [](auto) { callback_success_single_field = true; });

struct test_project_not_single_field {
    constexpr static auto config =
        cib::config(cib::exports<test_service>,
                    cib::extend<test_service>(test_callback_not_single_field));
};
} // namespace

TEST_CASE("build handler not single field", "[indexed_builder]") {
    cib::nexus<test_project_not_single_field> test_nexus{};
    test_nexus.init();

    callback_success_single_field = false;
    cib::service<test_service>->handle(
        test_msg_t{"test_id_field"_field = 0x50});
    CHECK(not callback_success_single_field);

    log_buffer.clear();
    callback_success_single_field = false;
    cib::service<test_service>->handle(
        test_msg_t{"test_id_field"_field = 0x51});
    CHECK(callback_success_single_field);
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("(collapsed to [true])") != std::string::npos);
}

namespace {
constexpr auto test_callback_not_multi_field =
    msg::callback<"test_callback_multi_field", msg_defn>(
        not test_id_field::in<0x80, 0x42> and test_opcode_field::equal_to<1>,
        [](auto) { callback_success = true; });

struct test_project_not_multi_field {
    constexpr static auto config =
        cib::config(cib::exports<test_service>,
                    cib::extend<test_service>(test_callback_not_multi_field,
                                              test_callback_not_single_field));
};
} // namespace

TEST_CASE("build handler not multi fields", "[indexed_builder]") {
    log_buffer.clear();
    cib::nexus<test_project_not_multi_field> test_nexus{};
    test_nexus.init();

    log_buffer.clear();
    callback_success = false;
    callback_success_single_field = false;
    cib::service<test_service>->handle(test_msg_t{
        "test_id_field"_field = 0x50, "test_opcode_field"_field = 1});
    CHECK(callback_success);
    CHECK(not callback_success_single_field);
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("(collapsed to [true])") != std::string::npos);

    log_buffer.clear();
    callback_success = false;
    callback_success_single_field = false;
    cib::service<test_service>->handle(
        test_msg_t{"test_id_field"_field = 0x51});
    CHECK(not callback_success);
    CHECK(callback_success_single_field);
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("(collapsed to [true])") != std::string::npos);

    log_buffer.clear();
    callback_success = false;
    callback_success_single_field = false;
    cib::service<test_service>->handle(test_msg_t{
        "test_id_field"_field = 0x51, "test_opcode_field"_field = 1});
    CHECK(callback_success);
    CHECK(callback_success_single_field);
    CAPTURE(log_buffer);
    CHECK(log_buffer.find("(collapsed to [true])") != std::string::npos);
}

namespace {
constexpr auto test_callback_disjunction =
    msg::callback<"test_callback_multi_field", msg_defn>(
        test_id_field::equal_to<0x80> or test_opcode_field::equal_to<1>,
        [](auto) { callback_success = true; });

struct test_project_disjunction {
    constexpr static auto config =
        cib::config(cib::exports<test_service>,
                    cib::extend<test_service>(test_callback_disjunction));
};
} // namespace

TEST_CASE("build handler disjunction", "[indexed_builder]") {
    cib::nexus<test_project_disjunction> test_nexus{};
    test_nexus.init();

    callback_success = false;
    cib::service<test_service>->handle(test_msg_t{
        "test_id_field"_field = 0x80, "test_opcode_field"_field = 1});
    CHECK(callback_success);

    callback_success = false;
    cib::service<test_service>->handle(test_msg_t{
        "test_id_field"_field = 0x80, "test_opcode_field"_field = 2});
    CHECK(callback_success);

    callback_success = false;
    cib::service<test_service>->handle(test_msg_t{
        "test_id_field"_field = 0x81, "test_opcode_field"_field = 1});
    CHECK(callback_success);
}

namespace {
using msg_match_defn = message<"test_msg", test_id_field::WithRequired<0x80>>;
using test_msg_match_t = owning<msg_match_defn>;

using msg_match_index_spec = msg::index_spec<test_id_field>;
struct test_msg_match_service
    : msg::indexed_service<msg_match_index_spec, test_msg_match_t> {};

constexpr auto test_msg_match_callback =
    msg::callback<"TestCallback", msg_match_defn>(
        test_id_field::equal_to<0x80>, [](auto) { callback_success = true; });

struct test_msg_match_project {
    constexpr static auto config = cib::config(
        cib::exports<test_msg_match_service>,
        cib::extend<test_msg_match_service>(test_msg_match_callback));
};
} // namespace

TEST_CASE("match output success (message matcher)", "[handler_builder]") {
    log_buffer.clear();
    cib::nexus<test_msg_match_project> test_nexus{};
    test_nexus.init();

    callback_success = false;
    cib::service<test_msg_match_service>->handle(
        test_msg_match_t{"test_id_field"_field = 0x80});
    CHECK(callback_success);

    callback_success = false;
    cib::service<test_msg_match_service>->handle(
        test_msg_match_t{"test_id_field"_field = 0x81});
    CHECK(not callback_success);
}

namespace {
constexpr auto test_callback_impossible =
    msg::callback<"test_callback_impossible", msg_defn>(
        test_id_field::equal_to<0x80> and test_id_field::equal_to<0x81>,
        [](auto) { callback_success = true; });

struct test_project_impossible {
    constexpr static auto config =
        cib::config(cib::exports<test_service>,
                    cib::extend<test_service>(test_callback_impossible));
};
} // namespace

TEST_CASE("build handler impossible matcher does not fail unless configured",
          "[indexed_builder]") {
    cib::nexus<test_project_impossible> test_nexus{};
    test_nexus.init();
}

namespace {
int callback_extra_arg{};

constexpr auto test_callback_extra_args =
    msg::callback<"test_callback_extra_args", msg_defn, int>(
        test_id_field::equal_to<0x80>, [](auto, int i) {
            callback_success = true;
            callback_extra_arg = i;
        });

struct test_service_extra_args : indexed_service<index_spec, test_msg_t, int> {
};
struct test_project_extra_args {
    constexpr static auto config = cib::config(
        cib::exports<test_service_extra_args>,
        cib::extend<test_service_extra_args>(test_callback_extra_args));
};
} // namespace

TEST_CASE("handle extra arguments", "[handler_builder]") {
    cib::nexus<test_project_extra_args> test_nexus{};
    test_nexus.init();

    cib::service<test_service_extra_args>->handle(
        test_msg_t{"test_id_field"_field = 0x80}, 42);
    CHECK(callback_success);
    CHECK(callback_extra_arg == 42);
}
