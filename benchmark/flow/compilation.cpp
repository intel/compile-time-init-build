// Compilation benchmark for the flow DSL.

#include "flow_graph.hpp"

#include <flow/graph_builder.hpp>

#include <utility>

namespace {
std::uint64_t volatile state{};

template <int Id> struct graph {
    constexpr static auto unique_action = [] { state = state + Id; };
    constexpr static auto value = flow_bench::make_graph(unique_action);
};

using bench_builder = flow::graph_builder<"compilation_flow_benchmark">;

template <int Id> void materialize() { bench_builder::render<graph<Id>>()(); }

template <int... Ids> void materialize_all(std::integer_sequence<int, Ids...>) {
    (materialize<Ids>(), ...);
}

constexpr auto graph_count = 8;
} // namespace

int main() { materialize_all(std::make_integer_sequence<int, graph_count>{}); }
