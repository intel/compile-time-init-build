#include <flow/debug_builder.hpp>
#include <flow/flow.hpp>
#include <nexus/config.hpp>
#include <nexus/nexus.hpp>

// EXPECT: _b --> _c

namespace {
using namespace flow::literals;

constexpr auto a = flow::action<"a">([] {});
constexpr auto b = flow::action<"b">([] {});
constexpr auto c = flow::action<"c">([] {});
constexpr auto d = flow::action<"d">([] {});

struct DebugFlow
    : public flow::service_for<
          flow::builder_for<flow::debug_builder<"b", "c", flow::mermaid>>> {};
struct DebugConfig {
    constexpr static auto config = cib::config(
        cib::exports<DebugFlow>, cib::extend<DebugFlow>(*a, *b, *c, *d),
        cib::extend<DebugFlow>(a >> b >> c >> d));
};
} // namespace

auto main() -> int {
    using namespace std::string_view_literals;
    cib::nexus<DebugConfig> nexus{};
    nexus.service<DebugFlow>();
}
