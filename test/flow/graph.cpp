#include <flow/flow.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>

namespace {
template <typename Graph>
[[nodiscard]] constexpr auto node_size(Graph const &g) -> std::size_t {
    return stdx::tuple_size_v<decltype(flow::dsl::get_nodes(g))>;
}

template <typename Graph>
[[nodiscard]] constexpr auto edge_size(Graph const &g) -> std::size_t {
    return flow::edge_size(flow::dsl::get_nodes(g), flow::dsl::get_edges(g));
}

constexpr auto a = flow::action<"a">([] {});
constexpr auto b = flow::action<"b">([] {});
constexpr auto c = flow::action<"c">([] {});
} // namespace

TEST_CASE("node size (empty graph)", "[flow]") {
    constexpr auto g = flow::graph<>{};
    static_assert(node_size(g) == 0);
}

TEST_CASE("node size (single action)", "[flow]") {
    constexpr auto g = flow::graph<>{}.add(a >> b);
    static_assert(node_size(g) == 2);
}

TEST_CASE("node size (overlapping actions)", "[flow]") {
    constexpr auto g = flow::graph<>{}.add(a >> b).add(b >> c);
    static_assert(node_size(g) == 3);
}

TEST_CASE("edge size (empty flow)", "[flow]") {
    constexpr auto g = flow::graph<>{};
    static_assert(edge_size(g) == 1);
}

TEST_CASE("edge size (single action)", "[flow]") {
    constexpr auto g = flow::graph<>{}.add(a >> b);
    static_assert(edge_size(g) == 1);
}

TEST_CASE("edge size (overlapping actions)", "[flow]") {
    constexpr auto g = flow::graph<>{}.add(a >> b).add(b >> c);
    static_assert(edge_size(g) == 2);
}
