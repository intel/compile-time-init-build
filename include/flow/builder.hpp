#pragma once

#include <cib/builder_meta.hpp>
#include <container/ConstexprMultiMap.hpp>
#include <flow/impl.hpp>
#include <flow/milestone.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace flow {
/**
 * flow::builder enables multiple independent components to collaboratively
 * build a flow::impl.
 *
 * flow::builder is fully constexpr and is not intended to ever execute at
 * runtime. Instead, flow::builder is intended to be executed in a constexpr
 * context to build Flows. The entire flow::impl building process can occur at
 * compile-time.
 *
 * @tparam NodeCapacity
 *      The maximum number of actions and milestones that can be added to a
 *      flow::builder.
 * *
 * @tparam DependencyCapacity
 *      The maximum number of dependencies from one action or milestone to
 *      another.
 *
 * @see flow::impl
 */
template <typename NodeType, template <typename, std::size_t> typename ImplType,
          typename NameT = void, std::size_t NodeCapacity = 64,
          std::size_t DependencyCapacity = 16>
class generic_builder {
  private:
    /**
     * A type to map milestone sources to milestone destinations. Like edges
     * on a graph.
     */
    using GraphType =
        ConstexprMultiMap<NodeType, NodeType, NodeCapacity, DependencyCapacity>;

    GraphType dependencyGraph;

    /**
     * @param graph
     *      The graph to check for edges.
     *
     * @param node
     *      The milestone to inspect.
     *
     * @return
     *      True if the milestone has no incoming edges.
     */
    static constexpr auto hasNoIncomingEdges(GraphType &graph, NodeType node)
        -> bool {
        for (auto const &entry : graph) {
            if (entry.value.contains(node)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr auto getNodesWithNoIncomingEdge() const
        -> ConstexprSet<NodeType, NodeCapacity> {
        GraphType graph = dependencyGraph;
        ConstexprSet<NodeType, NodeCapacity> nodesWithNoIncomingEdge;

        for (auto const &entry : graph) {
            nodesWithNoIncomingEdge.add(entry.key);
        }

        for (auto const &entry : graph) {
            for (auto const &dst : entry.value) {
                nodesWithNoIncomingEdge.remove(dst.key);
            }
        }

        return nodesWithNoIncomingEdge;
    }

    template <typename T>
    constexpr auto add_flow(T const &flow_description) -> void {
        if constexpr (std::is_base_of_v<NodeType, T>) {
            dependencyGraph.put(flow_description);
        } else {
            flow_description.walk([&](NodeType lhs, NodeType rhs) {
                if (rhs == NodeType{}) {
                    dependencyGraph.put(lhs);
                } else {
                    dependencyGraph.put(lhs, rhs);
                }
            });
        }
    }

  public:
    using Name = NameT;

    constexpr generic_builder() : dependencyGraph() {}

    /**
     * Add a flow description. A flow description describes the dependencies
     * between two or more milestones.
     *
     * For example, the following describes a flow with three milestones, each
     * one ordered after the previous:
     *
     * <pre>
     *      builder.add(a >> b >> c);
     * </pre>
     *
     * in_t this sequence milestones a, b, and c would be executed in that order
     * if the flow were directed to go to milestone c.
     *
     * detail::Milestones can be referenced in multiple calls to "add". For
     * example, the following specifies that both "b" and "c" will be executed
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
    template <typename... Ts>
    constexpr auto add(Ts const &...flow_descriptions) {
        (add_flow(flow_descriptions), ...);
        return *this;
    }

    /**
     * Create a flow::impl object combining all the specifications previously
     * given to the flow::builder.
     *
     * @tparam OptimizedFlowCapacity
     *      The maximum number of detail::Milestones and actions the flow::impl
     * will contain. This can be optimized to the minimal value if the
     *      flow::builder is assigned to a constexpr variable. The size()
     *      method can then be used as this template parameter.
     *
     * @return
     *      A flow::impl with all dependencies and requirements resolved.
     */
    template <std::size_t OptimizedFlowCapacity>
    [[nodiscard]] constexpr auto internal_build() const
        -> ImplType<Name, OptimizedFlowCapacity> {
        // https://en.wikipedia.org/wiki/Topological_sorting#Kahn's_algorithm
        GraphType graph = dependencyGraph;
        std::array<NodeType, NodeCapacity> orderedList{};
        std::size_t listSize = 0;

        auto nodesWithNoIncomingEdge = getNodesWithNoIncomingEdge();

        // while S is non-empty do
        while (!nodesWithNoIncomingEdge.isEmpty()) {
            // remove a node n from S
            auto n = nodesWithNoIncomingEdge.pop();

            // add n to tail of L
            orderedList[listSize] = n;
            listSize++;

            // for each node m with an edge e from n to m do
            if (graph.contains(n)) {
                auto ms = graph.get(n);
                if (ms.isEmpty()) {
                    graph.remove(n);

                } else {
                    for (auto entry : ms) {
                        auto m = entry.key;

                        // remove edge e from the graph
                        graph.remove(n, m);

                        // if m has no other incoming edges then
                        if (hasNoIncomingEdges(graph, m)) {
                            // insert m into S
                            nodesWithNoIncomingEdge.add(m);
                        }
                    }
                }
            }
        }

        /*
         * Tried moving this to return an error status with tl::expected, but it
         * caused a pretty significant increase in code size for little benefit.
         */
        auto buildStatus = graph.isEmpty()
                               ? build_status::SUCCESS
                               : build_status::FLOW_HAS_CIRCULAR_DEPENDENCY;

        return ImplType<Name, OptimizedFlowCapacity>(orderedList.data(),
                                                     buildStatus);
    }

    /**
     * @return
     *      The flow::impl Capacity necessary to fit the built flow::impl.
     */
    [[nodiscard]] constexpr auto size() const -> std::size_t {
        GraphType graph = dependencyGraph;
        ConstexprSet<NodeType, NodeCapacity> allNodes;

        for (auto entry : graph) {
            allNodes.add(entry.key);
            allNodes.addAll(entry.value);
        }

        return allNodes.getSize();
    }

    ///////////////////////////////////////////////////////////////////////////
    ///
    /// Everything below is for the cib extension interface. It lets cib know
    /// this builder supports the cib pattern and how to build it.
    ///
    ///////////////////////////////////////////////////////////////////////////
    using FunctionPtr = std::add_pointer<void()>::type;

    template <typename BuilderValue> static void runImpl() {
        constexpr auto builder = BuilderValue::value;
        constexpr auto size = builder.size();
        constexpr auto flow = builder.template internal_build<size>();

        static_assert(flow.getBuildStatus() == build_status::SUCCESS);

        flow();
    }

    template <typename BuilderValue>
    [[nodiscard]] static constexpr auto build() -> FunctionPtr {
        return runImpl<BuilderValue>;
    }
};

template <typename NameT = void, std::size_t NodeCapacity = 64,
          std::size_t DependencyCapacity = 16>
using builder = generic_builder<milestone_base, flow::impl, NameT, NodeCapacity,
                                DependencyCapacity>;

/**
 * Extend this to create named flow services.
 *
 * Types that extend flow::meta can be used as unique names with
 * cib::exports and cib::extend.
 *
 * @see cib::exports
 * @see cib::extend
 */
template <typename NameT = void, std::size_t NodeCapacity = 64,
          std::size_t DependencyCapacity = 16>
struct service
    : public cib::builder_meta<builder<NameT, NodeCapacity, DependencyCapacity>,
                               FunctionPtr> {};
} // namespace flow
