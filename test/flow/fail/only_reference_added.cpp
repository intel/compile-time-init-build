#include <flow/flow.hpp>
#include <nexus/config.hpp>
#include <nexus/nexus.hpp>

// EXPECT: One or more steps are referenced in the flow

namespace {
using namespace flow::literals;

constexpr auto a = flow::milestone<"a">();

struct TestFlowAlpha : public flow::service<"alpha"> {};

struct FlowConfig {
    constexpr static auto config =
        cib::config(cib::exports<TestFlowAlpha>, cib::extend<TestFlowAlpha>(a));
};
} // namespace

auto main() -> int {
    cib::nexus<FlowConfig> nexus{};
    nexus.service<TestFlowAlpha>();
}
