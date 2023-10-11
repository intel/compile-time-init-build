#include <cib/cib.hpp>
#include <log/fmt/logger.hpp>
#include <match/ops.hpp>
#include <msg/callback.hpp>
#include <msg/field.hpp>
#include <msg/message.hpp>
#include <msg/service.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <iterator>
#include <string>

namespace {
using test_id_field =
    msg::field<decltype("test_id_field"_sc), 0, 31, 24, std::uint32_t>;
using test_field_1 =
    msg::field<decltype("test_field_1"_sc), 0, 15, 0, std::uint32_t>;
using test_field_2 =
    msg::field<decltype("test_field_2"_sc), 1, 23, 16, std::uint32_t>;
using test_field_3 =
    msg::field<decltype("test_field_3"_sc), 1, 15, 0, std::uint32_t>;

using test_msg_t = msg::message_base<decltype("test_msg"_sc), 2,
                                     test_id_field::WithRequired<0x80>,
                                     test_field_1, test_field_2, test_field_3>;

struct test_service : msg::service<test_msg_t> {};

bool callback_success;

constexpr auto test_callback = msg::callback<test_msg_t>(
    "TestCallback"_sc, match::always,
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

TEST_CASE("build handler", "[handler_builder]") {
    cib::nexus<test_project> test_nexus{};
    test_nexus.init();

    callback_success = false;

    cib::service<test_service>->handle(test_msg_t{test_id_field{0x80}});

    REQUIRE(callback_success);
}

TEST_CASE("match output success", "[handler_builder]") {
    log_buffer.clear();
    cib::nexus<test_project> test_nexus{};
    test_nexus.init();
    cib::service<test_service>->handle(test_msg_t{test_id_field{0x80}});
    CHECK(log_buffer.find("Incoming message matched") != std::string::npos);
    CHECK(log_buffer.find("[TestCallback]") != std::string::npos);
    CHECK(log_buffer.find("[test_id_field == 0x80]") != std::string::npos);
}

TEST_CASE("match output failure", "[handler_builder]") {
    log_buffer.clear();
    cib::nexus<test_project> test_nexus{};
    test_nexus.init();
    cib::service<test_service>->handle(test_msg_t{test_id_field{0x81}});
    CHECK(log_buffer.find(
              "None of the registered callbacks claimed this message") !=
          std::string::npos);
    CHECK(log_buffer.find("TestCallback") != std::string::npos);
    CHECK(log_buffer.find("test_id_field (0x81) == 0x80") != std::string::npos);
}
