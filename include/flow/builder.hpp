#pragma once


#include <flow/milestone.hpp>
#include <flow/impl.hpp>

#include <container/ConstexprMultiMap.hpp>


namespace flow {
    /**
     * flow::builder enables multiple independent modules to collaboratively build
     * a flow::impl.
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
    template<typename NameT = void, std::size_t NodeCapacity = 64, std::size_t DependencyCapacity = 16>
    class Builder {
    private:
        /**
         * A type to map milestone sources to milestone destinations. Like edges
         * on a graph.
         */
        using GraphType =
            ConstexprMultiMap<milestone_base const *, milestone_base const *, NodeCapacity, DependencyCapacity>;

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
        static constexpr bool hasNoIncomingEdges(
            GraphType & graph,
            milestone_base const * node
        ) {
            // std::find_if is not constexpr in c++17 :(
            for (auto s : graph) {
                if (s.value.contains(node)) {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] constexpr ConstexprSet<milestone_base const *, NodeCapacity> getNodesWithNoIncomingEdge() const {
            GraphType graph = dependencyGraph;
            ConstexprSet<milestone_base const *, NodeCapacity> nodesWithNoIncomingEdge;

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

        constexpr Builder()
            : dependencyGraph()
        {}

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
         * In this sequence milestones a, b, and c would be executed in that order
         * if the flow were directed to go to milestone c.
         *
         * detail::Milestones can be referenced in multiple calls to "add". For example,
         * the following specifies that both "b" and "c" will be executed between
         * "a" and "d":
         *
         * <pre>
         *      builder.add(a >> b >> d);
         *      builder.add(a >> c >> d);
         * </pre>
         *
         * Note that it does not specify an ordering requirement between "b" and
         * "c".
         */
        template<typename T>
        constexpr void add(T const & flowDescription) {
            if constexpr(std::is_base_of_v<milestone_base, T>) {
                dependencyGraph.put(&flowDescription);

            } else {
                flowDescription.walk(
                    [&](milestone_base const * lhs, milestone_base const * rhs) {
                        if (rhs == nullptr) {
                            dependencyGraph.put(lhs);
                        } else {
                            dependencyGraph.put(lhs, rhs);
                        }
                    }
                );
            }
        }

        template<typename T1, typename T2, typename... Tn>
        constexpr void add(T1 const &flowDesc1, T2 const &flowDesc2, Tn const &... flowDescN) {
            add(flowDesc1);
            add(flowDesc2, flowDescN...);
        }


        /**
         * Create a flow::impl object combining all the specifications previously given
         * to the flow::builder.
         *
         * @tparam OptimizedFlowCapacity
         *      The maximum number of detail::Milestones and actions the flow::impl will
         *      contain. This can be optimized to the minimal value if the
         *      flow::builder is assigned to a constexpr variable. The size()
         *      method can then be used as this template parameter.
         *
         * @return
         *      A flow::impl with all dependencies and requirements resolved.
         */
        template<int OptimizedFlowCapacity>
        [[nodiscard]] constexpr flow::impl<Name, OptimizedFlowCapacity> internalBuild() const {
            // https://en.wikipedia.org/wiki/Topological_sorting#Kahn's_algorithm
            GraphType graph = dependencyGraph;
            milestone_base const *orderedList[NodeCapacity] = {};
            int listSize = 0;

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
             * Tried moving this to return an error status with tl::expected, but it caused a pretty
             * significant increase in code size for little benefit.
             */
            auto buildStatus =
                graph.isEmpty()
                    ? build_status::SUCCESS
                    : build_status::FLOW_HAS_CIRCULAR_DEPENDENCY;

            return flow::impl<Name, OptimizedFlowCapacity>(orderedList, buildStatus);
        }

        /**
         * @return
         *      The flow::impl Capacity necessary to fit the built flow::impl.
         */
        [[nodiscard]] constexpr std::size_t size() const {
            GraphType graph = dependencyGraph;
            ConstexprSet<milestone_base const *, NodeCapacity> allNodes;

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

        /**
         * Never called, but the return type is used by cib to determine what the
         * abstract interface is.
         */
        FunctionPtr base() const;

        template<typename BuilderValue>
        static void runImpl() {
            constexpr auto builder = BuilderValue::value;
            constexpr auto size = builder.size();
            constexpr auto flow = builder.template internalBuild<size>();

            static_assert(flow.getBuildStatus() == build_status::SUCCESS);

            flow();
        }

        template<typename BuilderValue>
        [[nodiscard]]  static constexpr FunctionPtr build() {
            return runImpl<BuilderValue>;
        }
    };
}
