#include <cib/cib.hpp>
#include <flow/flow.hpp>

// EXPECT: The conditions on this sequence

namespace {
using namespace flow::literals;

constexpr auto a = flow::milestone<"a">();
constexpr auto b = flow::milestone<"b">();

struct TestFlowAlpha : public flow::service<> {};

constexpr auto when_feature_a_enabled =
    cib::runtime_condition<"feature_a_enabled">([] { return true; });

struct FlowConfig {
    constexpr static auto config =
        cib::config(cib::exports<TestFlowAlpha>,
                    when_feature_a_enabled(cib::extend<TestFlowAlpha>(*a)),
                    cib::extend<TestFlowAlpha>(a >> *b));
};
} // namespace

auto main() -> int {
    cib::nexus<FlowConfig> nexus{};
    nexus.service<TestFlowAlpha>();
}
