#include <cib/cib.hpp>
#include <log/fmt/logger.hpp>
#include <msg/callback.hpp>
#include <msg/field.hpp>
#include <msg/message.hpp>
#include <msg/service.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <iterator>
#include <string>

namespace {
using namespace msg;

using id_field = field<"id", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;
using field1 = field<"f1", std::uint32_t>::located<at{0_dw, 15_msb, 0_lsb}>;
using field2 = field<"f2", std::uint32_t>::located<at{1_dw, 23_msb, 16_lsb}>;
using field3 = field<"f3", std::uint32_t>::located<at{1_dw, 15_msb, 0_lsb}>;

using msg_defn = message<"msg", id_field, field1, field2, field3>;
using test_msg_t = msg::owning<msg_defn>;
using msg_view_t = msg::const_view<msg_defn>;

constexpr auto id_match = msg::equal_to<id_field, 0x80>;

bool callback_success;

constexpr auto test_callback = msg::callback<"cb", msg_defn>(
    id_match, [](msg_view_t) { callback_success = true; });

struct test_service : msg::service<msg_view_t> {};
struct test_project {
    constexpr static auto config = cib::config(
        cib::exports<test_service>, cib::extend<test_service>(test_callback));
};

std::string log_buffer{};
} // namespace

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(log_buffer)};

TEST_CASE("build handler", "[handler_builder]") {
    cib::nexus<test_project> test_nexus{};
    test_nexus.init();

    callback_success = false;
    cib::service<test_service>->handle(test_msg_t{"id"_field = 0x80});
    CHECK(callback_success);
}

TEST_CASE("match output success", "[handler_builder]") {
    log_buffer.clear();
    cib::nexus<test_project> test_nexus{};
    test_nexus.init();

    cib::service<test_service>->handle(test_msg_t{"id"_field = 0x80});

    CAPTURE(log_buffer);
    CHECK(log_buffer.find("Incoming message matched") != std::string::npos);
    CHECK(log_buffer.find("[cb]") != std::string::npos);
    CHECK(log_buffer.find("[id == 0x80]") != std::string::npos);
}

TEST_CASE("match output failure", "[handler_builder]") {
    log_buffer.clear();
    cib::nexus<test_project> test_nexus{};
    test_nexus.init();

    cib::service<test_service>->handle(test_msg_t{"id"_field = 0x81});

    CAPTURE(log_buffer);
    CHECK(log_buffer.find(
              "None of the registered callbacks claimed this message") !=
          std::string::npos);
    CHECK(log_buffer.find("cb") != std::string::npos);
    CHECK(log_buffer.find("id (0x81) == 0x80") != std::string::npos);
}

namespace {
int callback_extra_arg{};

constexpr auto test_callback_extra_args =
    msg::callback<"cb", msg_defn, int>(id_match, [](msg_view_t, int i) {
        callback_success = true;
        callback_extra_arg = i;
    });

struct test_service_extra_args : msg::service<msg_view_t, int> {};
struct test_project_extra_args {
    constexpr static auto config = cib::config(
        cib::exports<test_service_extra_args>,
        cib::extend<test_service_extra_args>(test_callback_extra_args));
};
} // namespace

TEST_CASE("handle extra arguments", "[handler_builder]") {
    cib::nexus<test_project_extra_args> test_nexus{};
    test_nexus.init();

    cib::service<test_service_extra_args>->handle(test_msg_t{"id"_field = 0x80},
                                                  42);
    CHECK(callback_success);
    CHECK(callback_extra_arg == 42);
}
