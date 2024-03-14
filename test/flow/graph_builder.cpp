#include <flow/flow.hpp>
#include <flow/graphviz_builder.hpp>

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

using builder = flow::graph_builder<flow::impl>;
} // namespace

TEST_CASE("build and run empty flow", "[graph_builder]") {
    auto g = flow::graph<>{};
    auto const flow = builder::build(g);
    REQUIRE(flow.has_value());
    flow.value()();
}

TEST_CASE("add single action", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}.add(a);
    auto const flow = builder::build(g);
    REQUIRE(flow.has_value());
    flow.value()();
    CHECK(actual == "a");
}

TEST_CASE("two milestone linear before dependency", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}.add(a >> milestone0);
    auto const flow = builder::build(g);
    REQUIRE(flow.has_value());
    flow.value()();
    CHECK(actual == "a");
}

TEST_CASE("actions get executed once", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}
                 .add(a >> milestone0)
                 .add(a >> milestone1)
                 .add(milestone0 >> milestone1);
    auto const flow = builder::build(g);
    flow.value()();
    CHECK(actual == "a");
}

TEST_CASE("two milestone linear after dependency", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}
                 .add(a >> milestone0)
                 .add(milestone0 >> milestone1)
                 .add(milestone0 >> b >> milestone1);
    auto const flow = builder::build(g);
    flow.value()();
    CHECK(actual == "ab");
}

TEST_CASE("three milestone linear before and after dependency",
          "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}.add(a >> b >> c);
    auto const flow = builder::build(g);
    flow.value()();
    CHECK(actual == "abc");
}

TEST_CASE("just two actions in order", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}.add(a >> b);
    auto const flow = builder::build(g);
    flow.value()();
    CHECK(actual == "ab");
}

TEST_CASE("insert action between two actions", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}.add(a >> c).add(a >> b >> c);
    auto const flow = builder::build(g);
    flow.value()();
    CHECK(actual == "abc");
}

TEST_CASE("add single parallel 2", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}.add(a && b);
    auto const flow = builder::build(g);
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.size() == 2);
}

TEST_CASE("add single parallel 3", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}.add(a && b && c);
    auto const flow = builder::build(g);
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.size() == 3);
}

TEST_CASE("add single parallel 3 with later dependency 1", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}.add(a && b && c).add(c >> a);
    auto const flow = builder::build(g);
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('c') < actual.find('a'));
    CHECK(actual.size() == 3);
}

TEST_CASE("add single parallel 3 with later dependency 2", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}.add(a && b && c).add(a >> c);
    auto const flow = builder::build(g);
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('a') < actual.find('c'));
    CHECK(actual.size() == 3);
}

TEST_CASE("add parallel rhs", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}.add(a >> (b && c));
    auto const flow = builder::build(g);
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('a') < actual.find('b'));
    CHECK(actual.find('a') < actual.find('c'));
    CHECK(actual.size() == 3);
}

TEST_CASE("add parallel lhs", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}.add((a && b) >> c);
    auto const flow = builder::build(g);
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('a') < actual.find('c'));
    CHECK(actual.find('b') < actual.find('c'));
    CHECK(actual.size() == 3);
}

TEST_CASE("add parallel in the middle", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}.add(a >> (b && c) >> d);
    auto const flow = builder::build(g);
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

TEST_CASE("add dependency lhs", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}.add((a >> b) && c);
    auto const flow = builder::build(g);
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);

    CHECK(actual.find('a') < actual.find('b'));

    CHECK(actual.size() == 3);
}

TEST_CASE("add dependency rhs", "[graph_builder]") {
    actual.clear();
    auto g = flow::graph<>{}.add(a && (b >> c));
    auto const flow = builder::build(g);
    flow.value()();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);

    CHECK(actual.find('b') < actual.find('c'));

    CHECK(actual.size() == 3);
}

TEST_CASE("alternate builder", "[graph_builder]") {
    using alt_builder = flow::graphviz_builder;
    auto g = flow::graph<"debug">{}.add(a && (b >> c));
    auto const flow = alt_builder::build(g);
    auto expected = std::string{
        R"__debug__(digraph debug {
start -> a
start -> b
b -> c
a -> end
c -> end
})__debug__"};
    CHECK(flow == expected);
}
