#pragma once

#include <flow/dsl/walk.hpp>
#include <flow/graph_common.hpp>
#include <flow/viz_builder.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/static_assert.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <boost/mp11/set.hpp>

#include <type_traits>

namespace flow {
template <stdx::ct_string Start, stdx::ct_string End,
          typename Renderer = graphviz>
struct debug_builder {
    template <typename Graph>
    [[nodiscard]] static auto build(Graph const &input) {
        auto const nodes = stdx::to_unsorted_set(flow::dsl::get_nodes(input));
        auto const edges = stdx::to_unsorted_set(flow::dsl::get_edges(input));
        auto const named_nodes = stdx::apply_indices<name_for>(nodes);

        using nodes_t = std::remove_cvref_t<decltype(nodes)>;
        using edges_t = std::remove_cvref_t<decltype(edges)>;

        auto const start = stdx::get<stdx::cts_t<Start>>(named_nodes);
        using start_t = std::remove_cvref_t<decltype(start)>;

        auto const end = stdx::get<stdx::cts_t<End>>(named_nodes);
        using end_t = std::remove_cvref_t<decltype(end)>;

        using downstream_t = boost::mp11::mp_copy_if_q<
            nodes_t, detail::descendant_of_q<start_t, edges_t>>;
        using upstream_t =
            boost::mp11::mp_copy_if_q<nodes_t,
                                      detail::ancestor_of_q<end_t, edges_t>>;

        using subgraph_nodes_t =
            boost::mp11::mp_set_intersection<downstream_t, upstream_t>;
        using subgraph_edges_t = boost::mp11::mp_copy_if_q<
            edges_t, detail::contained_edge_q<subgraph_nodes_t>>;

        using viz_t = viz_builder<name, Renderer>;
        STATIC_ASSERT(
            false, "subgraph debug\n\n{}",
            (viz_t::template visualize<subgraph_nodes_t, subgraph_edges_t>()));
    }

    constexpr static auto name = Start + stdx::ct_string{" -> "} + End;
    using interface_t = auto (*)() -> void;

    template <typename Initialized> class built_flow {
        static auto run() -> void {
            auto const v = Initialized::value;
            build(v);
        }

      public:
        // NOLINTNEXTLINE(google-explicit-constructor)
        constexpr explicit(false) operator interface_t() const { return run; }
        auto operator()() const { run(); }
        constexpr static bool active = true;
    };

    template <typename Initialized>
    [[nodiscard]] constexpr static auto render() -> built_flow<Initialized> {
        return {};
    }
};
} // namespace flow
