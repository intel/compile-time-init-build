#include <cib/cib.hpp>
#include <flow/flow.hpp>

// EXPECT: Topological sort failed: cycle in flow

namespace {
using namespace flow::literals;

constexpr auto a = flow::milestone<"a">();
constexpr auto b = flow::milestone<"b">();

struct TestFlowAlpha : public flow::service<> {};

struct CyclicFlowConfig {
    constexpr static auto config = cib::config(
        cib::exports<TestFlowAlpha>, cib::extend<TestFlowAlpha>(a >> b >> a));
};
} // namespace

auto main() -> int {
    cib::nexus<CyclicFlowConfig> nexus{};
    nexus.service<TestFlowAlpha>();
}
