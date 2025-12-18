#include <flow/flow.hpp>
#include <log/level.hpp>
#include <nexus/config.hpp>
#include <nexus/nexus.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace {
using namespace flow::literals;

template <auto... Cs> struct wrapper {
    struct inner {
        constexpr static auto config = cib::config(Cs...);
    };
    constexpr static auto n = cib::nexus<inner>{};

    wrapper() { n.init(); }

    template <typename S> static auto run() -> void { n.template service<S>(); }
};

std::vector<logging::level> log_calls{};

template <typename S, auto... Cs>
constexpr auto run_flow = []() -> void {
    log_calls.clear();
    wrapper<Cs...>::template run<S>();
};

struct NamedTestFlow : public flow::service<"TestFlow"> {};

constexpr auto ms = flow::milestone<"ms">();

struct log_config {
    struct log_handler {
        template <typename Env, typename FilenameStringType,
                  typename LineNumberType, typename MsgType>
        auto log(FilenameStringType, LineNumberType, MsgType const &) -> void {
            log_calls.push_back(logging::get_level(Env{}));
        }
    };
    log_handler logger;
};
} // namespace

template <> inline auto logging::config<> = log_config{};

TEST_CASE("flow logs at TRACE by default", "[flow_log_levels]") {
    run_flow<NamedTestFlow, cib::exports<NamedTestFlow>,
             cib::extend<NamedTestFlow>(*ms)>();

    REQUIRE(not log_calls.empty());
    std::for_each(std::begin(log_calls), std::end(log_calls),
                  [](auto level) { CHECK(level == logging::level::TRACE); });
}
