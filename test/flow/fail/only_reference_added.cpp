#include <cib/cib.hpp>
#include <flow/flow.hpp>

// EXPECT: One or more steps are referenced in the flow but not explicitly

namespace {
using namespace flow::literals;

constexpr auto a = flow::milestone<"a">();

struct TestFlowAlpha : public flow::service<> {};

struct FlowConfig {
    constexpr static auto config =
        cib::config(cib::exports<TestFlowAlpha>, cib::extend<TestFlowAlpha>(a));
};
} // namespace

auto main() -> int {
    cib::nexus<FlowConfig> nexus{};
    nexus.service<TestFlowAlpha>();
}
