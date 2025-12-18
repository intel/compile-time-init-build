#include <flow/flow.hpp>
#include <nexus/config.hpp>
#include <nexus/nexus.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

std::string actual = {};

namespace {
using namespace flow::literals;

struct TestFlowAlpha : public flow::service<> {};

template <auto... Cs> struct wrapper {
    struct inner {
        constexpr static auto config = cib::config(Cs...);
    };
    constexpr static auto n = cib::nexus<inner>{};

    wrapper() { n.init(); }

    template <typename S> static auto run() -> void { n.template service<S>(); }
};

template <typename S, auto... Cs>
constexpr auto check_flow = [](std::string_view expected) -> void {
    actual.clear();
    wrapper<Cs...>::template run<S>();
    CHECK(actual == expected);
};
} // namespace

TEST_CASE("add actions using func_decl through cib::nexus",
          "[flow_separate_actions]") {
    check_flow<TestFlowAlpha, cib::exports<TestFlowAlpha>,
               cib::extend<TestFlowAlpha>(*"e"_action >> *"f"_action)>("ef");
}
