#include <cib/cib.hpp>
#include <flow/flow.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {
auto actual = std::string{};

constexpr auto a = flow::action<"a">([] { actual += "a"; });
constexpr auto b = flow::action<"b">([] { actual += "b"; });
constexpr auto c = flow::action<"c">([] { actual += "c"; });
constexpr auto d = flow::action<"d">([] { actual += "d"; });

struct TestFlowAlpha : public flow::service<> {};
struct TestFlowBeta : public flow::service<> {};

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
