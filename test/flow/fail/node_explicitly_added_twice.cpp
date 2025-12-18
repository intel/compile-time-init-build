#include <flow/flow.hpp>
#include <nexus/config.hpp>
#include <nexus/nexus.hpp>

// EXPECT: are explicitly added more than once

namespace {
using namespace flow::literals;

constexpr auto a = flow::milestone<"a">();

struct TestFlowAlpha : public flow::service<"alpha"> {};

struct FlowConfig {
    constexpr static auto config = cib::config(
        cib::exports<TestFlowAlpha>, cib::extend<TestFlowAlpha>(*a, *a));
};
} // namespace

auto main() -> int {
    cib::nexus<FlowConfig> nexus{};
    nexus.service<TestFlowAlpha>();
}
