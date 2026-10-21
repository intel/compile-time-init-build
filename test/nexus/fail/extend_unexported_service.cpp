#include <nexus/callback.hpp>
#include <nexus/config.hpp>
#include <nexus/nexus.hpp>

// EXPECT: Trying to extend a service \(foo\) that is not exported

namespace {
struct cb : public callback::service<> {
    constexpr static auto name = stdx::ct_string{"foo"};
};

struct extend_config {
    constexpr static auto config = cib::config(cib::extend<cb>([]() {}));
};
} // namespace

auto main() -> int {
    cib::nexus<extend_config> nexus{};
    nexus.init();
}
