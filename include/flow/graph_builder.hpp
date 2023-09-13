#pragma once

#include <flow/common.hpp>

#include <stdx/cx_multimap.hpp>
#include <stdx/cx_vector.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <optional>
#include <span>

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
    using graph_t = stdx::cx_multimap<Node, Node, NodeCapacity, EdgeCapacity>;
    graph_t graph{};

    [[nodiscard]] constexpr static auto is_source_of(Node node,
                                                     graph_t const &g) -> bool {
        return std::find_if(g.begin(), g.end(), [&](auto const &entry) {
                   return entry.value.contains(node);
               }) == g.end();
    }

    [[nodiscard]] constexpr auto get_sources() const
        -> stdx::cx_set<Node, NodeCapacity> {
        stdx::cx_set<Node, NodeCapacity> s;
        for (auto const &entry : graph) {
            s.insert(entry.key);
        }
        for (auto const &entry : graph) {
            for (auto const &dst : entry.value) {
                s.erase(dst);
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
        static_assert(built.has_value());
        built.value()();
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
    [[nodiscard]] constexpr auto topo_sort() const
        -> std::optional<Output<Name, Capacity>> {
        graph_t g = graph;
        stdx::cx_vector<Node, NodeCapacity> ordered_list{};

        auto sources = get_sources();
        while (not sources.empty()) {
            auto n = sources.pop_back();
            ordered_list.push_back(n);

            if (g.contains(n)) {
                auto ms = g.get(n);
                if (ms.empty()) {
                    g.erase(n);
                } else {
                    for (auto entry : ms) {
                        g.erase(n, entry);
                        if (is_source_of(entry, g)) {
                            sources.insert(entry);
                        }
                    }
                }
            }
        }

        if (not g.empty()) {
            return {};
        }
        return std::optional<Output<Name, Capacity>>{
            std::in_place,
            std::span{std::cbegin(ordered_list), std::size(ordered_list)}};
    }

    /**
     * @return The capacity necessary to fit the built graph.
     */
    [[nodiscard]] constexpr auto size() const -> std::size_t {
        stdx::cx_set<Node, NodeCapacity> all_nodes{};
        for (auto entry : graph) {
            all_nodes.insert(entry.key);
            all_nodes.merge(entry.value);
        }
        return all_nodes.size();
    }

    template <typename BuilderValue>
    [[nodiscard]] constexpr static auto build() -> FunctionPtr {
        return run_impl<BuilderValue, Derived::template impl_t>;
    }
};
} // namespace flow
