#include <cib/cib.hpp>
#include <flow/flow.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {
auto actual = std::string{};

constexpr auto milestone0 = flow::milestone<"milestone0">();
constexpr auto milestone1 = flow::milestone<"milestone1">();

constexpr auto a = flow::action<"a">([] { actual += "a"; });
constexpr auto b = flow::action<"b">([] { actual += "b"; });
constexpr auto c = flow::action<"c">([] { actual += "c"; });
constexpr auto d = flow::action<"d">([] { actual += "d"; });
} // namespace

TEST_CASE("node size (empty flow)", "[flow]") {
    constexpr auto builder = flow::builder<>{};
    static_assert(builder.node_size() == 0);
}

TEST_CASE("node size (single action)", "[flow]") {
    constexpr auto builder = flow::builder<>{}.add(a >> b);
    static_assert(builder.node_size() == 2);
}

TEST_CASE("node size (overlapping actions)", "[flow]") {
    constexpr auto builder = flow::builder<>{}.add(a >> b).add(b >> c);
    static_assert(builder.node_size() == 3);
}

TEST_CASE("edge size (empty flow)", "[flow]") {
    constexpr auto builder = flow::builder<>{};
    static_assert(builder.edge_size() == 1);
}

TEST_CASE("edge size (single action)", "[flow]") {
    constexpr auto builder = flow::builder<>{}.add(a >> b);
    static_assert(builder.edge_size() == 1);
}

TEST_CASE("edge size (overlapping actions)", "[flow]") {
    constexpr auto builder = flow::builder<>{}.add(a >> b).add(b >> c);
    static_assert(builder.edge_size() == 2);
}

TEST_CASE("build and run empty flow", "[flow]") {
    auto builder = flow::builder<>{};
    auto const flow = builder.topo_sort<flow::impl>();
    REQUIRE(flow.has_value());
    flow.value()();
}

TEST_CASE("add single action", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}.add(a);
    auto const flow = builder.topo_sort<flow::impl>();
    REQUIRE(flow.has_value());
    flow.value()();
    CHECK(actual == "a");
}

TEST_CASE("two milestone linear before dependency", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}.add(a >> milestone0);
    auto const flow = builder.topo_sort<flow::impl>();
    REQUIRE(flow.has_value());
    flow.value()();
    CHECK(actual == "a");
}

TEST_CASE("actions get executed once", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}
                       .add(a >> milestone0)
                       .add(a >> milestone1)
                       .add(milestone0 >> milestone1);
    auto const flow = builder.topo_sort<flow::impl>();
    flow.value()();
    CHECK(actual == "a");
}

TEST_CASE("two milestone linear after dependency", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}
                       .add(a >> milestone0)
                       .add(milestone0 >> milestone1)
                       .add(milestone0 >> b >> milestone1);
    auto const flow = builder.topo_sort<flow::impl>();
    flow.value()();
    CHECK(actual == "ab");
}

TEST_CASE("three milestone linear before and after dependency", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}.add(a >> b >> c);
    auto const flow = builder.topo_sort<flow::impl>();
    flow.value()();
    CHECK(actual == "abc");
}

TEST_CASE("just two actions in order", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}.add(a >> b);
    auto const flow = builder.topo_sort<flow::impl>();
    flow.value()();
    CHECK(actual == "ab");
}

TEST_CASE("insert action between two actions", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}.add(a >> c).add(a >> b >> c);
    auto const flow = builder.topo_sort<flow::impl>();
    flow.value()();
    CHECK(actual == "abc");
}

TEST_CASE("add single parallel 2", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}.add(a && b);
    auto const flow = builder.topo_sort<flow::impl>();
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.size() == 2);
}

TEST_CASE("add single parallel 3", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}.add(a && b && c);
    auto const flow = builder.topo_sort<flow::impl>();
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.size() == 3);
}

TEST_CASE("add single parallel 3 with later dependency 1", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}.add(a && b && c).add(c >> a);
    auto const flow = builder.topo_sort<flow::impl>();
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('c') < actual.find('a'));
    CHECK(actual.size() == 3);
}

TEST_CASE("add single parallel 3 with later dependency 2", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}.add(a && b && c).add(a >> c);
    auto const flow = builder.topo_sort<flow::impl>();
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('a') < actual.find('c'));
    CHECK(actual.size() == 3);
}

TEST_CASE("add parallel rhs", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}.add(a >> (b && c));
    auto const flow = builder.topo_sort<flow::impl>();
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('a') < actual.find('b'));
    CHECK(actual.find('a') < actual.find('c'));
    CHECK(actual.size() == 3);
}

TEST_CASE("add parallel lhs", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}.add((a && b) >> c);
    auto const flow = builder.topo_sort<flow::impl>();
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('a') < actual.find('c'));
    CHECK(actual.find('b') < actual.find('c'));
    CHECK(actual.size() == 3);
}

TEST_CASE("add parallel in the middle", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}.add(a >> (b && c) >> d);
    auto const flow = builder.topo_sort<flow::impl>();
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('d') != std::string::npos);

    CHECK(actual.find('a') < actual.find('b'));
    CHECK(actual.find('a') < actual.find('c'));

    CHECK(actual.find('b') < actual.find('d'));
    CHECK(actual.find('c') < actual.find('d'));

    CHECK(actual.size() == 4);
}

TEST_CASE("add dependency lhs", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}.add((a >> b) && c);
    auto const flow = builder.topo_sort<flow::impl>();
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);

    CHECK(actual.find('a') < actual.find('b'));

    CHECK(actual.size() == 3);
}

TEST_CASE("add dependency rhs", "[flow]") {
    actual.clear();
    auto builder = flow::builder<>{}.add(a && (b >> c));
    auto const flow = builder.topo_sort<flow::impl>();
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);

    CHECK(actual.find('b') < actual.find('c'));

    CHECK(actual.size() == 3);
}

namespace {
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
