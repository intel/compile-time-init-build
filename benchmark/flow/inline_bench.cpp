#include "flow_graph.hpp"

#include <flow/graph_builder.hpp>

#include <cstdint>
#include <iostream>

namespace {
std::uint64_t count{};
std::uint64_t volatile *count_ptr = &count;

struct graph {
    constexpr static auto value =
        flow_bench::make_graph([] { *count_ptr = 1; });
};

using bench_builder = flow::graph_builder<"inline_flow_benchmark">;
} // namespace

int main() {
    asm volatile("start_write_label:\n");
    *count_ptr = 1;
    asm volatile("end_write_label:\n");
    extern char start_write_label, end_write_label;
    std::ptrdiff_t size_of_write = &end_write_label - &start_write_label;

    asm volatile("start_size_label:\n");
    auto const flow = bench_builder::render<graph>();
    flow();
    asm volatile("end_size_label:\n");

    extern char start_size_label, end_size_label;
    std::ptrdiff_t code_size = &end_size_label - &start_size_label;
    auto bytes_per_node = code_size / flow_bench::graph_node_count;

    if (bytes_per_node != size_of_write) {
        std::cerr << "Expected " << size_of_write << " bytes per node but got "
                  << bytes_per_node << ".\n";
        return 1;
    }

    return 0;
}
