#pragma once

#include <container/constexpr_multimap.hpp>
#include <flow/common.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>

namespace flow {
namespace detail {
template <typename T, typename Node>
concept walkable = std::same_as<T, Node> or
                   requires(T const &t) { t.walk([](Node, Node) {}); };
}

/**
 * flow::graph_builder allows a compile-time graph to be built, and then used to
 * output a topologically sorted flow.
 *
 * flow::graph_builder is fully constexpr and is not intended to ever execute
 * at runtime. Instead, flow::graph_builder is intended to be executed in a
 * constexpr context to build Flows. The entire building process can occur at
 * compile-time.
 *
 * @tparam Node The type of a flow node.
 * @tparam Name The name of this builder.
 * @tparam NodeCapacity The maximum number of nodes that can be added.
 * @tparam EdgeCapacity The maximum number of edges between one node and
 *         another.
 * @tparam Derived The class that uses graph_builder with CRTP.
 */
template <typename Node, typename NameT, std::size_t NodeCapacity,
          std::size_t EdgeCapacity, typename Derived>
class graph_builder {
    using graph_t =
        cib::constexpr_multimap<Node, Node, NodeCapacity, EdgeCapacity>;
    graph_t graph{};

    [[nodiscard]] constexpr static auto is_source_of(Node node,
                                                     graph_t const &g) -> bool {
        return std::find_if(g.begin(), g.end(), [&](auto const &entry) {
                   return entry.value.contains(node);
               }) == g.end();
    }

    [[nodiscard]] constexpr auto get_sources() const
        -> cib::constexpr_set<Node, NodeCapacity> {
        cib::constexpr_set<Node, NodeCapacity> s;
        for (auto const &entry : graph) {
            s.add(entry.key);
        }
        for (auto const &entry : graph) {
            for (auto const &dst : entry.value) {
                s.remove(dst.key);
            }
        }
        return s;
    }

    constexpr auto insert(Node const &node) -> void { graph.put(node); }

    template <detail::walkable<Node> T>
    constexpr auto insert(T const &t) -> void {
        t.walk([&](Node lhs, Node rhs) {
            if (rhs == Node{}) {
                graph.put(lhs);
            } else {
                graph.put(lhs, rhs);
            }
        });
    }

    template <typename BuilderValue,
              template <typename, std::size_t> typename Output>
    static auto run_impl() -> void {
        constexpr auto builder = BuilderValue::value;
        constexpr auto size = builder.size();
        constexpr auto built = builder.template topo_sort<Output, size>();
        static_assert(built.getBuildStatus() == flow::build_status::SUCCESS);
        built();
    }

  public:
    using Name = NameT;

    /**
     * Add flow descriptions. A flow description describes the dependencies
     * between two or more nodes.
     *
     * For example, the following describes a flow with three nodes, each
     * one ordered after the previous:
     *
     * <pre>
     *      builder.add(a >> b >> c);
     * </pre>
     *
     * In this sequence node a, b, and c would be visited in that order
     * if the flow were directed to go to node c.
     *
     * Nodes can be referenced in multiple calls to "add". For example,
     * the following specifies that both "b" and "c" will be visited
     * between "a" and "d":
     *
     * <pre>
     *      builder.add(a >> b >> d);
     *      builder.add(a >> c >> d);
     * </pre>
     *
     * Note that it does not specify an ordering requirement between "b" and
     * "c".
     */
    template <detail::walkable<Node>... Ts>
    constexpr auto add(Ts const &...descriptions) -> Derived & {
        (insert(descriptions), ...);
        return static_cast<Derived &>(*this);
    }

    /**
     * Create an object combining all the specifications previously given to the
     * builder.
     *
     * @tparam Output The (template) type of the output object.
     * @tparam Capacity The maximum number of nodes the object will contain.
     * This can be optimized to the minimal value if the builder is assigned to
     * a constexpr variable. The size() method can then be used as this template
     * parameter.
     *
     * @return An object with all dependencies and requirements resolved.
     */
    template <template <typename, std::size_t> typename Output,
              std::size_t Capacity>
    [[nodiscard]] constexpr auto topo_sort() const -> Output<Name, Capacity> {
        graph_t g = graph;
        std::array<Node, NodeCapacity> ordered_list{};
        std::size_t list_size{};

        auto sources = get_sources();
        while (not sources.empty()) {
            auto n = sources.pop();
            ordered_list[list_size++] = n;

            if (g.contains(n)) {
                auto ms = g.get(n);
                if (ms.empty()) {
                    g.remove(n);
                } else {
                    for (auto entry : ms) {
                        auto m = entry.key;
                        g.remove(n, m);
                        if (is_source_of(m, g)) {
                            sources.add(m);
                        }
                    }
                }
            }
        }

        auto buildStatus = g.empty() ? build_status::SUCCESS
                                     : build_status::HAS_CIRCULAR_DEPENDENCY;
        return Output<Name, Capacity>(ordered_list.data(), buildStatus);
    }

    /**
     * @return The capacity necessary to fit the built graph.
     */
    [[nodiscard]] constexpr auto size() const -> std::size_t {
        cib::constexpr_set<Node, NodeCapacity> all_nodes{};
        for (auto entry : graph) {
            all_nodes.add(entry.key);
            all_nodes.add_all(entry.value);
        }
        return all_nodes.size();
    }

    template <typename BuilderValue>
    [[nodiscard]] constexpr static auto build() -> FunctionPtr {
        return run_impl<BuilderValue, Derived::template impl_t>;
    }
};
} // namespace flow
