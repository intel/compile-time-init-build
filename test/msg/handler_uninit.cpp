#include <msg/message.hpp>
#include <msg/service.hpp>
#include <nexus/service.hpp>

#include <stdx/panic.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>
#include <string_view>

struct raw_msg_t {};

namespace {
using namespace msg;

using msg_defn = message<"msg">;
using test_msg_t = msg::owning<msg_defn>;
using msg_view_t = msg::const_view<msg_defn>;
struct test_service : msg::service<msg_view_t> {};
struct raw_service : msg::service<raw_msg_t> {};

std::string panic_string = {};
int panics{};

struct test_panic_handler {
    template <stdx::ct_string Why, typename... Ts>
    static auto panic(Ts &&...) -> void {
        panic_string = std::string_view{Why};
        ++panics;
    }
};
} // namespace

template <> inline auto stdx::panic_handler<> = test_panic_handler{};

TEST_CASE("invoke handle on service when uninitialized (named msg type)",
          "[handler]") {
    panics = 0;
    cib::service<test_service>->handle(test_msg_t{});
    CHECK(panics == 1);
    CHECK(panic_string ==
          "Attempting to handle msg (msg) before service is initialized");
}

TEST_CASE("invoke handle on service when uninitialized (raw msg type)",
          "[handler]") {
    panics = 0;
    cib::service<raw_service>->handle(raw_msg_t{});
    CHECK(panics == 1);
    CHECK(panic_string ==
          "Attempting to handle msg (raw_msg_t) before service is initialized");
}
