#pragma once

#include <flow/dsl/walk.hpp>
#include <flow/graph_common.hpp>
#include <flow/step.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/utility.hpp>

#include <algorithm>
#include <string_view>
#include <type_traits>
#include <utility>

namespace flow {
struct graphviz {
    template <stdx::ct_string Name> CONSTEVAL static auto prologue() {
        return +stdx::ct_format<"digraph {} {">(stdx::cts_t<Name>{});
    }
    template <stdx::ct_string Name> CONSTEVAL static auto epilogue() {
        using namespace stdx::literals;
        return "}"_cts;
    }
    template <typename Node> CONSTEVAL static auto render_node(Node) {
        return +stdx::ct_format<"  {}">(typename Node::name_t{});
    }
    template <typename Edge> CONSTEVAL static auto render_edge(Edge) {
        return +stdx::ct_format<"  {} -> {}">(typename Edge::source_t::name_t{},
                                              typename Edge::dest_t::name_t{});
    }
};

class mermaid {
    template <stdx::ct_string Name> CONSTEVAL static auto to_id() {
        using namespace stdx::literals;
        return "_"_ctst + stdx::cts_t<[] {
                   auto n = Name;
                   std::replace(n.begin(), n.end(), ' ', '_');
                   return n;
               }()>{};
    }

    template <typename Node> CONSTEVAL static auto to_box() {
        using namespace stdx::literals;
        if constexpr (detail::is_milestone<Node>) {
            return stdx::ct_format<"[{}]">(typename Node::name_t{});
        } else {
            return stdx::ct_format<"({})">(typename Node::name_t{});
        }
    }

  public:
    template <stdx::ct_string Name> CONSTEVAL static auto prologue() {
        return +stdx::ct_format<"---\ntitle: {}\n---\nflowchart TD">(
            stdx::cts_t<Name>{});
    }

    template <stdx::ct_string Name> CONSTEVAL static auto epilogue() {
        using namespace stdx::literals;
        return ""_cts;
    }

    template <typename Node> CONSTEVAL static auto render_node(Node) {
        return +stdx::ct_format<"  {}{}">(to_id<Node::name_t::value>(),
                                          to_box<Node>());
    }

    template <typename Node>
        requires(Node::name_t::value == stdx::ct_string{"start"} or
                 Node::name_t::value == stdx::ct_string{"end"})
    CONSTEVAL static auto render_node(Node) {
        return +stdx::ct_format<"  {}(({}))">(to_id<Node::name_t::value>(),
                                              typename Node::name_t{});
    }

    template <typename Edge> CONSTEVAL static auto render_edge(Edge) {
        using namespace stdx::literals;

        if constexpr (Edge::cond_t::ct_name == "always"_cts) {
            return +stdx::ct_format<"  {} --> {}">(
                to_id<Edge::source_t::name_t::value>(),
                to_id<Edge::dest_t::name_t::value>());
        } else {
            return +stdx::ct_format<"  {} --{}--> {}">(
                to_id<Edge::source_t::name_t::value>(),
                stdx::cts_t<Edge::cond_t::ct_name>{},
                to_id<Edge::dest_t::name_t::value>());
        }
    }
};

template <stdx::ct_string Name, typename Renderer = graphviz>
struct viz_builder {
    template <typename Nodes, typename Edges>
    [[nodiscard]] constexpr static auto visualize() {
        using namespace stdx::literals;
        auto prologue = Renderer::template prologue<Name>();
        auto epilogue = Renderer::template epilogue<Name>();
        auto rendered_nodes = stdx::transform(
            [](auto n) { return Renderer::render_node(n); }, Nodes{});
        auto rendered_edges = stdx::transform(
            [](auto e) { return Renderer::render_edge(e); }, Edges{});

        return prologue + "\n"_cts +
               std::move(rendered_nodes)
                   .join(""_cts,
                         [](auto &&x, auto &&y) {
                             return FWD(x) + "\n"_cts + FWD(y);
                         }) +
               "\n"_cts +
               std::move(rendered_edges)
                   .join(""_cts,
                         [](auto &&x, auto &&y) {
                             return FWD(x) + "\n"_cts + FWD(y);
                         }) +
               "\n"_cts + epilogue;
    }

    template <typename Graph>
    [[nodiscard]] constexpr static auto build(Graph const &input) {
        using flow::operator""_milestone;

        auto const nodes = stdx::to_sorted_set(flow::dsl::get_nodes(input));
        auto const edges = stdx::to_sorted_set(flow::dsl::get_edges(input));
        using edges_t = std::remove_cvref_t<decltype(edges)>;

        auto const sources = stdx::filter<
            detail::has_no_edge_q<detail::matching_dest, edges_t>::template fn>(
            nodes);
        auto const sinks =
            stdx::filter<detail::has_no_edge_q<detail::matching_source,
                                               edges_t>::template fn>(nodes);

        constexpr auto start = "start"_milestone;
        constexpr auto end = "end"_milestone;
        using start_t = std::remove_cvref_t<decltype(start)>;
        using end_t = std::remove_cvref_t<decltype(end)>;

        constexpr auto edge_from_start = []<typename N>(N) {
            return dsl::edge<start_t, N, typename N::cond_t>{};
        };
        constexpr auto edge_to_end = []<typename N>(N) {
            return dsl::edge<N, end_t, typename N::cond_t>{};
        };

        auto const renderable_nodes =
            stdx::tuple_cat(stdx::tuple{start, end}, nodes);
        auto const renderable_edges =
            stdx::tuple_cat(edges, stdx::transform(edge_from_start, sources),
                            stdx::transform(edge_to_end, sinks));
        using renderable_nodes_t =
            std::remove_cvref_t<decltype(renderable_nodes)>;
        using renderable_edges_t =
            std::remove_cvref_t<decltype(renderable_edges)>;

        return visualize<renderable_nodes_t, renderable_edges_t>();
    }

    constexpr static auto name = Name;
    using interface_t = auto (*)() -> std::string_view;

    template <typename Initialized> class built_flow {
        constexpr static auto built() {
            auto const v = Initialized::value;
            return build(v);
        }
        constexpr static auto built_v = built();

        constexpr static auto run() -> std::string_view {
            return std::string_view{built_v};
        }

      public:
        // NOLINTNEXTLINE(google-explicit-constructor)
        constexpr explicit(false) operator interface_t() const { return run; }
        constexpr auto operator()() const { return run(); }
        constexpr static bool active = true;
    };

    template <typename Initialized>
    [[nodiscard]] constexpr static auto render() -> built_flow<Initialized> {
        return {};
    }
};
} // namespace flow
