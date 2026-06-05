// Runtime benchmark for the flow DSL.
// Checking that things get inlined.

#define ANKERL_NANOBENCH_IMPLEMENT

#include "flow_graph.hpp"

#include <flow/graph_builder.hpp>

#include <cstdint>

#include <nanobench.h>

namespace {
std::uint64_t count{};
std::uint64_t volatile *count_ptr = &count;

struct graph {
    constexpr static auto value =
        flow_bench::make_graph([] { ++(*count_ptr); });
};

using bench_builder = flow::graph_builder<"run_flow_benchmark">;
} // namespace

int main() {
    auto const flow = bench_builder::render<graph>();

    ankerl::nanobench::Bench().minEpochIterations(2000000).run("flow run",
                                                               [&] { flow(); });

    ankerl::nanobench::doNotOptimizeAway(count);
}
