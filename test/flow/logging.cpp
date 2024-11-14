#include <cib/cib.hpp>
#include <flow/flow.hpp>
#include <log/fmt/logger.hpp>

#include <catch2/catch_test_macros.hpp>

#include <iterator>
#include <string>

namespace {
using namespace flow::literals;

std::string log_buffer{};

template <auto... Cs> struct wrapper {
    struct inner {
        constexpr static auto config = cib::config(Cs...);
    };
    constexpr static auto n = cib::nexus<inner>{};

    wrapper() { n.init(); }

    template <typename S> static auto run() -> void { n.template service<S>(); }
};

template <typename S, auto... Cs>
constexpr auto run_flow = [] {
    log_buffer.clear();
    wrapper<Cs...>::template run<S>();
};

struct TestFlowAlpha : public flow::service<> {};
struct NamedTestFlow : public flow::service<"TestFlow"> {};

constexpr auto a = flow::action<"a">([] {});
} // namespace

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(log_buffer)};

TEST_CASE("unnamed flow does not log start/end", "[flow_logging]") {
    run_flow<TestFlowAlpha, cib::exports<TestFlowAlpha>>();

    CHECK(std::empty(log_buffer));
}

TEST_CASE("empty flow logs start/end", "[flow_logging]") {
    run_flow<NamedTestFlow, cib::exports<NamedTestFlow>>();

    CHECK(log_buffer.find("flow.start(TestFlow)") != std::string::npos);
    CHECK(log_buffer.find("flow.end(TestFlow)") != std::string::npos);
}

TEST_CASE("unnamed flow does not log actions", "[flow_logging]") {
    run_flow<TestFlowAlpha, cib::exports<TestFlowAlpha>,
             cib::extend<TestFlowAlpha>(*a)>();

    CHECK(log_buffer.find("flow.action(a)") == std::string::npos);
}

TEST_CASE("named flow logs actions", "[flow_logging]") {
    run_flow<NamedTestFlow, cib::exports<NamedTestFlow>,
             cib::extend<NamedTestFlow>(*a)>();

    CHECK(log_buffer.find("flow.action(a)") != std::string::npos);
}

TEST_CASE("named flow logs milestones", "[flow_logging]") {
    run_flow<NamedTestFlow, cib::exports<NamedTestFlow>,
             cib::extend<NamedTestFlow>(*"ms"_milestone)>();

    CHECK(log_buffer.find("flow.milestone(ms)") != std::string::npos);
}
