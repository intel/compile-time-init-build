#include <cib/cib.hpp>
#include <cib/func_decl.hpp>
#include <flow/flow.hpp>
#include <log/fmt/logger.hpp>

#include <catch2/catch_test_macros.hpp>

#include <iterator>
#include <string>

std::string actual = {};

namespace {
constexpr auto a = flow::action<"a">([] { actual += "a"; });
constexpr auto b = flow::action<"b">([] { actual += "b"; });
constexpr auto c = flow::action<"c">([] { actual += "c"; });
constexpr auto d = flow::action<"d">([] { actual += "d"; });
constexpr auto e = flow::action<"e">();

constexpr auto ms = flow::milestone<"ms">();

struct TestFlowAlpha : public flow::service<> {};
struct TestFlowBeta : public flow::service<> {};

struct SingleFlowEmptyConfig {
    constexpr static auto config = cib::config(cib::exports<TestFlowAlpha>);
};
} // namespace

TEST_CASE("run empty flow through cib::nexus", "[flow]") {
    actual.clear();
    cib::nexus<SingleFlowEmptyConfig> nexus{};
    nexus.service<TestFlowAlpha>();
    CHECK(std::empty(actual));
}

namespace {
struct SingleFlowSingleActionConfig {
    constexpr static auto config =
        cib::config(cib::exports<TestFlowAlpha>, cib::extend<TestFlowAlpha>(a));
};
} // namespace

TEST_CASE("add single action through cib::nexus", "[flow]") {
    actual.clear();
    cib::nexus<SingleFlowSingleActionConfig> nexus{};
    nexus.service<TestFlowAlpha>();
    CHECK(actual == "a");
}

namespace {
struct MultiFlowMultiActionConfig {
    constexpr static auto config = cib::config(
        cib::exports<TestFlowAlpha, TestFlowBeta>,
        cib::extend<TestFlowAlpha>(a >> b), cib::extend<TestFlowBeta>(c >> d));
};
} // namespace

TEST_CASE("add multi action through cib::nexus", "[flow]") {
    actual.clear();
    cib::nexus<MultiFlowMultiActionConfig> nexus{};
    nexus.service<TestFlowAlpha>();
    nexus.service<TestFlowBeta>();
    CHECK(actual == "abcd");
}

TEST_CASE("add multi action through cib::nexus, run through cib::service",
          "[flow]") {
    actual.clear();
    cib::nexus<MultiFlowMultiActionConfig> nexus{};
    nexus.init();
    cib::service<TestFlowAlpha>();
    cib::service<TestFlowBeta>();
    CHECK(actual == "abcd");
}

namespace {
struct SingleFlowSingleActionFuncDeclConfig {
    constexpr static auto config =
        cib::config(cib::exports<TestFlowAlpha>, cib::extend<TestFlowAlpha>(e));
};
} // namespace

TEST_CASE("add single action using func_decl through cib::nexus", "[flow]") {
    actual.clear();
    cib::nexus<SingleFlowSingleActionFuncDeclConfig> nexus{};
    nexus.service<TestFlowAlpha>();
    CHECK(actual == "e");
}

namespace {
struct NamedTestFlow : public flow::service<"TestFlow"> {};

struct EmptyNamedFlowConfig {
    constexpr static auto config = cib::config(cib::exports<NamedTestFlow>);
};

struct NamedFlowConfig {
    constexpr static auto config =
        cib::config(cib::exports<NamedTestFlow>, cib::extend<NamedTestFlow>(a));
};

struct MSNamedFlowConfig {
    constexpr static auto config = cib::config(cib::exports<NamedTestFlow>,
                                               cib::extend<NamedTestFlow>(ms));
};

std::string log_buffer{};
} // namespace

template <>
inline auto logging::config<> =
    logging::fmt::config{std::back_inserter(log_buffer)};

TEST_CASE("unnamed flow does not log start/end", "[flow]") {
    log_buffer.clear();
    cib::nexus<SingleFlowEmptyConfig> nexus{};
    nexus.service<TestFlowAlpha>();
    CHECK(std::empty(log_buffer));
}

TEST_CASE("empty flow logs start/end", "[flow]") {
    log_buffer.clear();
    cib::nexus<EmptyNamedFlowConfig> nexus{};
    nexus.service<NamedTestFlow>();
    CHECK(log_buffer.find("flow.start(TestFlow)") != std::string::npos);
    CHECK(log_buffer.find("flow.end(TestFlow)") != std::string::npos);
}

TEST_CASE("unnamed flow does not log actions", "[flow]") {
    log_buffer.clear();
    cib::nexus<SingleFlowSingleActionConfig> nexus{};
    nexus.service<TestFlowAlpha>();
    CHECK(log_buffer.find("flow.action(a)") == std::string::npos);
}

TEST_CASE("named flow logs actions", "[flow]") {
    log_buffer.clear();
    cib::nexus<NamedFlowConfig> nexus{};
    nexus.service<NamedTestFlow>();
    CHECK(log_buffer.find("flow.action(a)") != std::string::npos);
}

TEST_CASE("named flow logs milestones", "[flow]") {
    log_buffer.clear();
    cib::nexus<MSNamedFlowConfig> nexus{};
    nexus.service<NamedTestFlow>();
    CHECK(log_buffer.find("flow.milestone(ms)") != std::string::npos);
}
