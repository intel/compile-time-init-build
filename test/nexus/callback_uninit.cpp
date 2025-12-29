#include <nexus/callback.hpp>
#include <nexus/service.hpp>

#include <stdx/panic.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

namespace {
template <int Id, typename... Args>
struct TestCallback : public callback::service<Args...> {};

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

TEST_CASE("run callback service when uninitialized", "[callback]") {
    panics = 0;
    cib::service<TestCallback<0>>();
    CHECK(panics == 1);
    CHECK(panic_string ==
          "Attempting to run callback before it is initialized");
}
