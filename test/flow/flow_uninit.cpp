#include <flow/service.hpp>
#include <nexus/service.hpp>

#include <stdx/panic.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

namespace {
struct TestFlow : flow::service<"test"> {};

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

TEST_CASE("run flow when uninitialized", "[flow]") {
    panics = 0;
    cib::service<TestFlow>();
    CHECK(panics == 1);
    CHECK(panic_string ==
          "Attempting to run flow (test) before it is initialized");
}
