#pragma once

#include <flow/dsl/walk.hpp>
#include <flow/step.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/utility.hpp>

#include <algorithm>
#include <array>
#include <iterator>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace flow {
struct graphviz {
    template <typename Graph> static auto prologue() -> std::string {
        std::string output{"digraph "};
        output += std::string_view{Graph::name};
        output += " {";
        return output;
    }
    template <typename Graph> static auto epilogue() -> std::string {
        return "}";
    }
    template <typename Node> static auto render_node(Node) -> std::string {
        std::string output{"  "};
        output += std::string_view{Node::name_t::value};
        return output;
    }
    template <typename Edge> static auto render_edge(Edge) -> std::string {
        std::string output{"  "};
        output += std::string_view{Edge::source_t::name_t::value};
        output += " -> ";
        output += std::string_view{Edge::dest_t::name_t::value};
        return output;
    }
};

class mermaid {
    static auto to_id(auto str) {
        std::string s{"_"};
        s += str;
        std::replace(std::begin(s), std::end(s), ' ', '_');
        return s;
    }

  public:
    template <typename Graph> static auto prologue() -> std::string {
        std::string output{"---\ntitle: "};
        output += std::string_view{Graph::name};
        output += "\n---\nflowchart TD";
        return output;
    }

    template <typename Graph> static auto epilogue() -> std::string {
        return "";
    }

    template <typename Node> static auto render_node(Node) -> std::string {
        using namespace std::string_view_literals;
        std::string output{"  "};
        constexpr auto node_opener = std::array{"("sv, "["sv};
        constexpr auto node_closer = std::array{")"sv, "]"sv};
        output += to_id(std::string_view{Node::name_t::value});
        output += node_opener[detail::is_milestone<Node>];
        output += std::string_view{Node::name_t::value};
        output += node_closer[detail::is_milestone<Node>];
        return output;
    }

    template <typename Node>
        requires(Node::name_t::value == stdx::ct_string{"start"} or
                 Node::name_t::value == stdx::ct_string{"end"})
    static auto render_node(Node) -> std::string {
        using namespace std::string_view_literals;
        std::string output{"  "};
        output += to_id(std::string_view{Node::name_t::value});
        output += "((";
        output += std::string_view{Node::name_t::value};
        output += "))";
        return output;
    }

    template <typename Edge> static auto render_edge(Edge) -> std::string {
        std::string output{"  "};
        output += to_id(std::string_view{Edge::source_t::name_t::value});
        output += " --";
        if constexpr (Edge::cond_t::ct_name != stdx::ct_string{"always"}) {
            output += std::string_view{Edge::cond_t::ct_name};
            output += "--";
        }
        output += "> ";
        output += to_id(std::string_view{Edge::dest_t::name_t::value});
        return output;
    }
};

namespace detail {
template <typename N> struct matching_source {
    template <typename Edge>
    using fn = std::is_same<N, typename Edge::source_t>;
};
template <typename N> struct matching_dest {
    template <typename Edge> using fn = std::is_same<N, typename Edge::dest_t>;
};

template <template <typename> typename Matcher, typename Edges>
struct has_no_edge_q {
    template <typename Node>
    using fn =
        boost::mp11::mp_empty<boost::mp11::mp_copy_if_q<Edges, Matcher<Node>>>;
};
} // namespace detail

template <stdx::ct_string Name, typename Renderer = graphviz>
struct viz_builder {
    template <typename Graph>
    [[nodiscard]] static auto build(Graph const &input) {
        using flow::operator""_milestone;

        auto const nodes = stdx::to_sorted_set(flow::dsl::get_nodes(input));
        auto const edges = stdx::to_sorted_set(flow::dsl::get_edges(input));
        using nodes_t = std::remove_cvref_t<decltype(nodes)>;
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

        auto prologue = Renderer::template prologue<Graph>();
        auto epilogue = Renderer::template epilogue<Graph>();
        auto rendered_nodes = stdx::transform(
            [](auto n) { return Renderer::render_node(n); }, renderable_nodes);
        auto rendered_edges = stdx::transform(
            [](auto e) { return Renderer::render_edge(e); }, renderable_edges);

        return prologue + '\n' +
               std::move(rendered_nodes)
                   .join(std::string{},
                         [](auto &&x, auto &&y) {
                             return FWD(x) + '\n' + FWD(y);
                         }) +
               '\n' +
               std::move(rendered_edges)
                   .join(std::string{},
                         [](auto &&x, auto &&y) {
                             return FWD(x) + '\n' + FWD(y);
                         }) +
               '\n' + epilogue;
    }

    constexpr static auto name = Name;
    using interface_t = auto (*)() -> std::string;

    template <typename Initialized> class built_flow {
        static auto built() {
            auto const v = Initialized::value;
            return build(v);
        }

        static auto run() -> std::string { return built(); }

      public:
        // NOLINTNEXTLINE(google-explicit-constructor)
        constexpr explicit(false) operator interface_t() const { return run; }
        auto operator()() const { return run(); }
        constexpr static bool active = true;
    };

    template <typename Initialized>
    [[nodiscard]] constexpr static auto render() -> built_flow<Initialized> {
        return {};
    }
};
} // namespace flow
