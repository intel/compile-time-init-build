#include <msg/callback.hpp>
#include <msg/field.hpp>
#include <msg/message.hpp>
#include <msg/send.hpp>
#include <msg/service.hpp>
#include <nexus/config.hpp>
#include <nexus/nexus.hpp>

#include <async/schedulers/trigger_manager.hpp>
#include <async/sync_wait.hpp>

#include <stdx/ct_conversions.hpp>
#include <stdx/ct_string.hpp>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {
template <typename T>
constexpr auto type_string =
    stdx::ct_string<stdx::type_as_string<T>().size() + 1>{
        stdx::type_as_string<T>()};
} // namespace

TEMPLATE_TEST_CASE("request-response", "[send]", decltype([] {})) {
    constexpr auto name = type_string<TestType>;
    int var{};

    auto s = msg::send([&](auto i) { async::run_triggers<name>(i); }, 42) |
             msg::then_receive<name, int>(
                 [&](auto recvd, auto x) { var = recvd + x; }, 17);
    CHECK(var == 0);
    CHECK(async::sync_wait(s));
    CHECK(var == 59);
}

namespace {
using msg::at;
using msg::operator""_dw;
using msg::operator""_msb;
using msg::operator""_lsb;
using msg::operator""_f;

using id_field =
    msg::field<"id", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;
using field1 =
    msg::field<"f1", std::uint32_t>::located<at{0_dw, 15_msb, 0_lsb}>;
using field2 =
    msg::field<"f2", std::uint32_t>::located<at{1_dw, 23_msb, 16_lsb}>;
using field3 =
    msg::field<"f3", std::uint32_t>::located<at{1_dw, 15_msb, 0_lsb}>;

using msg_defn = msg::message<"msg", id_field, field1, field2, field3>;
using test_msg_t = msg::owning<msg_defn>;
using msg_view_t = msg::const_view<msg_defn>;

constexpr auto test_callback = msg::callback<"cb", msg_defn>(
    "id"_f == msg::constant<0x80>,
    [](msg_view_t v) { async::run_triggers<"cb">(v); });

struct test_service : msg::service<msg_view_t> {};
struct test_project {
    constexpr static auto config = cib::config(
        cib::exports<test_service>, cib::extend<test_service>(test_callback));
};
} // namespace

TEST_CASE("request-response through handler", "[send]") {
    cib::nexus<test_project> test_nexus{};
    test_nexus.init();

    std::uint32_t var{};
    auto s =
        msg::send(
            [&](auto id) {
                cib::service<test_service>->handle(test_msg_t{"id"_f = id});
            },
            0x80) |
        msg::then_receive<"cb", msg_view_t>(
            [&](auto v) { var = v.get("id"_f); });
    CHECK(async::sync_wait(s));
    CHECK(var == 0x80);
}
