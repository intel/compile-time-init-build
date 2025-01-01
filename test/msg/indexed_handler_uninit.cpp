#include <cib/built.hpp>
#include <msg/indexed_service.hpp>
#include <msg/message.hpp>

#include <stdx/panic.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

namespace {
using namespace msg;

using msg_defn = message<"msg">;
using test_msg_t = msg::owning<msg_defn>;
using msg_view_t = msg::const_view<msg_defn>;

using index_spec = msg::index_spec<>;
struct test_service : msg::indexed_service<index_spec, msg_view_t> {};

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

TEST_CASE("invoke handle on service when uninitialized", "[indexed_handler]") {
    panics = 0;
    cib::service<test_service>->handle(test_msg_t{});
    CHECK(panics == 1);
    CHECK(panic_string ==
          "Attempting to handle msg (msg) before service is initialized");
}
