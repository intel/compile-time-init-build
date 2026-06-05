// Compilation benchmark for the flow DSL.
//
// Used to measure how long the front-end spends instantiating the flow graph
// DSL.
//
// Build it with the compilation profiler:
//   ninja compilation_flow_benchmark
//   clang++ -ftime-trace ... benchmark/flow/compilation.cpp
// and inspect the -ftime-report / -ftime-trace output.

#include "flow_graph.hpp"

#include <flow/graph_builder.hpp>

#include <cstdint>
#include <utility>

namespace {
std::uint64_t volatile sink{};

template <int Id> struct graph {
    constexpr static auto tagged = [] { sink = sink + Id; };
    constexpr static auto value = flow_bench::make_graph(tagged);
};

using bench_builder = flow::graph_builder<"compilation_flow_benchmark">;

template <int Id> void materialize() { bench_builder::render<graph<Id>>()(); }

template <int... Ids> void materialize_all(std::integer_sequence<int, Ids...>) {
    (materialize<Ids>(), ...);
}

constexpr auto graph_count = 24;
} // namespace

int main() { materialize_all(std::make_integer_sequence<int, graph_count>{}); }
