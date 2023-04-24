#pragma once

#include <cib/builder_meta.hpp>
#include <container/ConstexprMultiMap.hpp>
#include <seq/build_status.hpp>
#include <seq/impl.hpp>
#include <seq/step.hpp>

#include <algorithm>

namespace seq {
template <typename NameT = void, std::size_t NodeCapacity = 64,
          std::size_t DependencyCapacity = 16>
class builder {
  private:
    /**
     * A type to map step sources to step destinations. Like edges
     * on a graph.
     */
    using GraphType = ConstexprMultiMap<step_base, step_base, NodeCapacity,
                                        DependencyCapacity>;

    GraphType dependencyGraph;

    /**
     * @param graph
     *      The graph to check for edges.
     *
     * @param node
     *      The step to inspect.
     *
     * @return
     *      True if the step has no incoming edges.
     */
    constexpr static auto hasNoIncomingEdges(GraphType &graph, step_base node)
        -> bool {
        return std::find_if(graph.begin(), graph.end(), [&](auto const &s) {
                   return s.value.contains(node);
               }) == graph.end();
    }

    [[nodiscard]] constexpr auto getNodesWithNoIncomingEdge() const
        -> ConstexprSet<step_base, NodeCapacity> {
        GraphType graph = dependencyGraph;
        ConstexprSet<step_base, NodeCapacity> nodesWithNoIncomingEdge;

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

  public:
    using Name = NameT;

    constexpr builder() : dependencyGraph() {}

    /**
     * Add a seq description. A seq description describes the dependencies
     * between two or more steps.
     *
     * For example, the following describes a seq with three steps, each
     * one ordered after the previous:
     *
     * <pre>
     *      builder.add(a >> b >> c);
     * </pre>
     *
     * in_t this sequence steps a, b, and c would be executed in that order
     * if the seq were directed to go to step c.
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
    template <typename T>
    constexpr auto add(T const &flowDescription) -> builder & {
        if constexpr (std::is_base_of_v<step_base, T>) {
            dependencyGraph.put(flowDescription);

        } else {
            flowDescription.walk([&](step_base lhs, step_base rhs) {
                if (rhs == step_base{}) {
                    dependencyGraph.put(lhs);
                } else {
                    dependencyGraph.put(lhs, rhs);
                }
            });
        }

        return *this;
    }

    /**
     * Create a seq::impl object combining all the specifications previously
     * given to the seq::builder.
     *
     * @tparam OptimizedFlowCapacity
     *      The maximum number of detail::Milestones and actions the seq::impl
     * will contain. This can be optimized to the minimal value if the
     *      seq::builder is assigned to a constexpr variable. The size()
     *      method can then be used as this template parameter.
     *
     * @return
     *      A seq::impl with all dependencies and requirements resolved.
     */
    template <std::size_t OptimizedFlowCapacity>
    [[nodiscard]] constexpr auto internal_build() const
        -> seq::impl<OptimizedFlowCapacity> {
        // https://en.wikipedia.org/wiki/Topological_sorting#Kahn's_algorithm
        GraphType graph = dependencyGraph;
        std::array<step_base, NodeCapacity> orderedList{};
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
                               : build_status::SEQ_HAS_CIRCULAR_DEPENDENCY;

        return seq::impl<OptimizedFlowCapacity>(orderedList.data(),
                                                buildStatus);
    }

    /**
     * @return
     *      The seq::impl Capacity necessary to fit the built seq::impl.
     */
    [[nodiscard]] constexpr auto size() const -> std::size_t {
        GraphType graph = dependencyGraph;
        ConstexprSet<step_base, NodeCapacity> allNodes;

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

    template <typename BuilderValue> static auto runImpl() -> void {
        constexpr auto builder = BuilderValue::value;
        constexpr auto size = builder.size();
        constexpr auto seq = builder.template internal_build<size>();

        static_assert(seq.getBuildStatus() == build_status::SUCCESS);

        seq();
    }

    template <typename BuilderValue>
    [[nodiscard]] constexpr static auto build() -> func_ptr {
        return runImpl<BuilderValue>;
    }
};

/**
 * Extend this to create named seq services.
 *
 * Types that extend seq::meta can be used as unique names with
 * cib::exports and cib::extend.
 *
 * @see cib::exports
 * @see cib::extend
 */
template <typename NameT = void, std::size_t NodeCapacity = 64,
          std::size_t DependencyCapacity = 16>
struct service
    : public cib::builder_meta<builder<NameT, NodeCapacity, DependencyCapacity>,
                               func_ptr> // FIXME: this needs to be updated to
                                         // use a pure virtual base class
{};
} // namespace seq
