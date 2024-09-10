#include <flow/flow.hpp>
#include <flow/graphviz_builder.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

namespace {
auto actual = std::string{};

constexpr auto milestone0 = flow::milestone<"milestone0">();
constexpr auto milestone1 = flow::milestone<"milestone1">();

constexpr auto a = flow::action<"a">([] { actual += "a"; });
constexpr auto b = flow::action<"b">([] { actual += "b"; });
constexpr auto c = flow::action<"c">([] { actual += "c"; });
constexpr auto d = flow::action<"d">([] { actual += "d"; });

using builder = flow::graph_builder<"test_flow", flow::impl>;
} // namespace

template <auto... Vs> struct run_flow_t {
    struct wrapper {
        constexpr static auto value = flow::graph<>{}.add(Vs...);
    };

    auto operator()() const -> void { builder::render<wrapper>()(); }
};

template <auto... Vs> constexpr auto run_flow = run_flow_t<Vs...>{};

template <auto... Vs> auto check_flow(std::string_view expected) -> void {
    actual.clear();
    run_flow<Vs...>();
    CHECK(actual == expected);
}

#if defined(__GNUC__) && __GNUC__ == 12
#else

TEST_CASE("build and run empty flow", "[graph_builder]") { check_flow<>(""); }

TEST_CASE("add single action", "[graph_builder]") { check_flow<*a>("a"); }

TEST_CASE("two milestone linear before dependency", "[graph_builder]") {
    check_flow<(*a >> *milestone0)>("a");
}

TEST_CASE("actions get executed once", "[graph_builder]") {
    check_flow<(*a >> *milestone0), (a >> *milestone1),
               (milestone0 >> milestone1)>("a");
}

TEST_CASE("two milestone linear after dependency", "[graph_builder]") {
    check_flow<(*a >> *milestone0), (milestone0 >> *milestone1),
               (milestone0 >> *b >> milestone1)>("ab");
}

TEST_CASE("three milestone linear before and after dependency",
          "[graph_builder]") {
    check_flow<(*a >> *b >> *c)>("abc");
}

TEST_CASE("just two actions in order", "[graph_builder]") {
    check_flow<(*a >> *b)>("ab");
}

TEST_CASE("operator* on two actions in order", "[graph_builder]") {
    check_flow<*(a >> b)>("ab");
}

TEST_CASE("operator* on three actions in order", "[graph_builder]") {
    check_flow<*(a >> b >> c)>("abc");
}

TEST_CASE("insert action between two actions", "[graph_builder]") {
    check_flow<(*a >> *c), (a >> *b >> c)>("abc");
}

TEST_CASE("add single parallel 2", "[graph_builder]") {
    actual.clear();
    run_flow<*a && *b>();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.size() == 2);
}

TEST_CASE("operator* on add single parallel 2", "[graph_builder]") {
    actual.clear();
    run_flow<*(a && b)>();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.size() == 2);
}

TEST_CASE("operator* on double parallel", "[graph_builder]") {
    actual.clear();
    run_flow<*(a && b && c)>();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.size() == 3);
}

TEST_CASE("add single parallel 3", "[graph_builder]") {
    actual.clear();
    run_flow<*a && *b && *c>();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.size() == 3);
}

TEST_CASE("add single parallel 3 with later dependency 1", "[graph_builder]") {
    actual.clear();
    run_flow<*a && *b && *c, (c >> a)>();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('c') < actual.find('a'));
    CHECK(actual.size() == 3);
}

TEST_CASE("add single parallel 3 with later dependency 2", "[graph_builder]") {
    actual.clear();
    run_flow<*a && *b && *c, (a >> c)>();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('a') < actual.find('c'));
    CHECK(actual.size() == 3);
}

TEST_CASE("add parallel rhs", "[graph_builder]") {
    actual.clear();
    run_flow<(*a >> (*b && *c))>();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('a') < actual.find('b'));
    CHECK(actual.find('a') < actual.find('c'));
    CHECK(actual.size() == 3);
}

TEST_CASE("add parallel lhs", "[graph_builder]") {
    actual.clear();
    run_flow<((*a && *b) >> *c)>();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('a') < actual.find('c'));
    CHECK(actual.find('b') < actual.find('c'));
    CHECK(actual.size() == 3);
}

TEST_CASE("add parallel in the middle", "[graph_builder]") {
    actual.clear();
    run_flow<(*a >> (*b && *c) >> *d)>();

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
    run_flow<(*a >> *b) && *c>();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);

    CHECK(actual.find('a') < actual.find('b'));

    CHECK(actual.size() == 3);
}

TEST_CASE("add dependency rhs", "[graph_builder]") {
    actual.clear();
    run_flow<*a && (*b >> *c)>();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);

    CHECK(actual.find('b') < actual.find('c'));

    CHECK(actual.size() == 3);
}

TEST_CASE("reference vs non-reference", "[graph_builder]") {
    static_assert(a.identity == flow::subgraph_identity::REFERENCE);
    static_assert((*a).identity == flow::subgraph_identity::VALUE);
}

TEST_CASE("reference in order with non-reference added twice",
          "[graph_builder]") {
    check_flow<(*a >> b) && (*b)>("ab");
}

#endif

TEST_CASE("alternate builder", "[graph_builder]") {
    using alt_builder = flow::graphviz_builder;
    auto g = flow::graph<"debug">{}.add(*a && (*b >> *c));
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
