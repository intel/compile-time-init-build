#include <nexus/config.hpp>
#include <nexus/nexus.hpp>
#include <nexus/service.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_conversions.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <string_view>
#include <tuple>
#include <utility>

namespace {
bool nexus_injected{};

template <typename B> struct test_service {
    using builder_t = B;
    using interface_t = typename B::interface_t;
    CONSTEVAL static auto uninitialized() -> interface_t { return {}; }
};

template <typename... Fs> struct test_builder {
    using interface_t = auto (*)() -> void;
    std::tuple<Fs...> funcs;

    template <typename... Args> constexpr auto add(Args... as) {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            return test_builder<Fs..., Args...>{
                .funcs = {get<Is>(funcs)..., as...}};
        }(std::make_index_sequence<sizeof...(Fs)>{});
    }

    template <typename BuilderValue, typename Nexus> static auto run() -> void {
        constexpr auto v = BuilderValue::value;
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            (get<Is>(v.funcs)(Nexus{}), ...);
        }(std::make_index_sequence<sizeof...(Fs)>{});
    }

    template <typename BuilderValue, typename Nexus>
    [[nodiscard]] CONSTEVAL static auto build() -> interface_t {
        return run<BuilderValue, Nexus>;
    }
};

using service_t = test_service<test_builder<>>;

struct test_config {
    constexpr static auto config =
        cib::config(cib::exports<service_t>,
                    cib::extend<service_t>(
                        []<typename Nexus>(Nexus) { nexus_injected = true; }));
};
} // namespace

TEST_CASE("injected nexus", "[injected_nexus_service]") {
    nexus_injected = false;
    cib::nexus<test_config> nexus{};
    nexus.init();
    cib::service<test_service<test_builder<>>>();
    CHECK(nexus_injected);
}
