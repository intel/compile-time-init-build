#pragma once

#include <flow/dsl/walk.hpp>
#include <flow/func_list.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/cx_multimap.hpp>
#include <stdx/cx_set.hpp>
#include <stdx/cx_vector.hpp>
#include <stdx/span.hpp>
#include <stdx/static_assert.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/utility.hpp>

#include <boost/mp11/set.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <optional>
#include <utility>

namespace flow {
namespace detail {
template <typename T> using is_duplicated = std::bool_constant<(T::size() > 1)>;

template <typename CTNode, typename Output>
concept is_output_compatible = requires(CTNode n) {
    { Output::create_node(n) } -> std::same_as<typename Output::node_t>;
};

template <typename T>
constexpr static auto error_steps =
    T{}.join(stdx::cts_t<"">{}, [](auto x, auto y) {
        using namespace stdx::literals;
        return x + ", "_ctst + y;
    });
} // namespace detail

template <typename T> using name_for = typename T::name_t;

[[nodiscard]] constexpr auto edge_size(auto const &nodes, auto const &edges)
    -> std::size_t {
    auto const edge_capacities = transform(
        [&]<typename N>(N const &) {
            return edges.fold_left(
                std::size_t{}, []<typename E>(auto acc, E const &) {
                    if constexpr (std::is_same_v<name_for<typename E::source_t>,
                                                 name_for<N>> or
                                  std::is_same_v<name_for<typename E::dest_t>,
                                                 name_for<N>>) {
                        return ++acc;
                    } else {
                        return acc;
                    }
                });
        },
        nodes);

    return edge_capacities.fold_left(std::size_t{1}, [](auto acc, auto next) {
        return std::max(acc, next);
    });
}

template <stdx::ct_string Name, typename LogPolicy = log_policy_t<Name>,
          template <stdx::ct_string, typename, std::size_t> typename Impl =
              func_list>
struct graph_builder {
    // NOLINTBEGIN(readability-function-cognitive-complexity)
    template <typename Output, std::size_t N, std::size_t E>
    [[nodiscard]] constexpr static auto make_graph(auto const &nodes,
                                                   auto const &edges) {
        using output_node_t = typename Output::node_t;
        using graph_t = stdx::cx_multimap<output_node_t, output_node_t, N, E>;
        graph_t g{};
        for_each([&]<typename Node>(Node n) { g.put(Output::create_node(n)); },
                 nodes);

        auto const named_nodes = stdx::apply_indices<name_for>(nodes);
        for_each(
            [&]<typename Lhs, typename Rhs, typename Cond>(
                dsl::edge<Lhs, Rhs, Cond> const &) {
                auto const lhs = get<name_for<Lhs>>(named_nodes);
                auto const rhs = get<name_for<Rhs>>(named_nodes);

                using lhs_t = std::remove_cvref_t<decltype(lhs)>;
                using rhs_t = std::remove_cvref_t<decltype(rhs)>;
                using lhs_cond_t = std::remove_cvref_t<decltype(lhs.condition)>;
                using rhs_cond_t = std::remove_cvref_t<decltype(rhs.condition)>;

                using edge_ps_t = decltype(Cond::predicates);
                constexpr auto node_ps = stdx::to_unsorted_set(stdx::tuple_cat(
                    lhs_t::condition.predicates, rhs_t::condition.predicates));

                stdx::for_each(
                    [&]<typename P>(P const &) {
                        STATIC_ASSERT(
                            (stdx::contains_type<edge_ps_t, P>),
                            "The conditions on the sequence ({} >> {})[{}] are "
                            "weaker than those on {}[{}] or {}[{}]. "
                            "Specifically, the sequence is missing the "
                            "predicate: {}",
                            lhs_t::ct_name, rhs_t::ct_name, Cond::ct_name,
                            lhs_t::ct_name, lhs_cond_t::ct_name, rhs_t::ct_name,
                            rhs_cond_t::ct_name, P);
                    },
                    node_ps);

                g.put(Output::create_node(lhs), Output::create_node(rhs));
            },
            edges);
        return g;
    }
    // NOLINTEND(readability-function-cognitive-complexity)

    template <typename Node, typename Graph>
    [[nodiscard]] constexpr static auto is_source_of(Node const &node,
                                                     Graph const &g) -> bool {
        return std::find_if(g.begin(), g.end(), [&](auto const &entry) {
                   return entry.value.contains(node);
               }) == g.end();
    }

    template <typename Graph>
    [[nodiscard]] constexpr static auto get_sources(Graph const &g)
        -> stdx::cx_set<typename Graph::key_type, Graph::capacity()> {
        stdx::cx_set<typename Graph::key_type, Graph::capacity()> s;
        for (auto const &entry : g) {
            s.insert(entry.key);
        }
        for (auto const &entry : g) {
            for (auto const &dst : entry.value) {
                s.erase(dst);
            }
        }
        return s;
    }

    template <typename Output, typename Graph>
    [[nodiscard]] constexpr static auto topo_sort(Graph &g)
        -> std::optional<Output> {
        stdx::cx_vector<typename Graph::key_type, Graph::capacity()>
            ordered_list{};

        auto sources = get_sources(g);
        while (not sources.empty()) {
            auto n = sources.pop_back();
            ordered_list.push_back(n);

            if (g.contains(n)) {
                auto ms = g.get(n);
                if (ms.empty()) {
                    g.erase(n);
                } else {
                    for (auto const &entry : ms) {
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
        using span_t =
            stdx::span<typename Graph::key_type const, Graph::capacity()>;
        return std::optional<Output>{std::in_place, span_t{ordered_list}};
    }

    // NOLINTNEXTLINE (readability-function-cognitive-complexity)
    constexpr static void check_for_missing_nodes(auto nodes,
                                                  auto mentioned_nodes) {
        constexpr auto get_name = []<typename N>(N) ->
            typename N::name_t { return {}; };
        auto node_names = stdx::transform(get_name, nodes);
        auto mentioned_node_names = stdx::transform(get_name, mentioned_nodes);

        using node_names_t = decltype(stdx::to_sorted_set(node_names));
        using mentioned_node_names_t =
            decltype(stdx::to_sorted_set(mentioned_node_names));
        using missing_nodes_t =
            boost::mp11::mp_set_difference<mentioned_node_names_t,
                                           node_names_t>;
        STATIC_ASSERT(
            (std::is_same_v<node_names_t, mentioned_node_names_t>),
            "One or more steps are referenced in the flow ({}) but not "
            "explicitly added with the * operator. The missing steps are: "
            "{}.",
            Name, detail::error_steps<missing_nodes_t>);

        constexpr auto duplicates = stdx::transform(
            [](auto e) { return stdx::get<0>(e); },
            stdx::filter<detail::is_duplicated>(stdx::gather(node_names)));
        using duplicate_nodes_t = decltype(duplicates);
        STATIC_ASSERT(
            stdx::tuple_size_v<duplicate_nodes_t> == 0,
            "One or more steps in the flow ({}) are explicitly added more than "
            "once using the * operator. The duplicate steps are: {}.",
            Name, detail::error_steps<duplicate_nodes_t>);
    }

    template <typename Graph>
    [[nodiscard]] constexpr static auto build(Graph const &input) {
        auto nodes = flow::dsl::get_nodes(input);
        auto mentioned_nodes = flow::dsl::get_all_mentioned_nodes(input);

        check_for_missing_nodes(nodes, mentioned_nodes);

        auto node_set = stdx::to_unsorted_set(nodes);
        auto edges = stdx::to_unsorted_set(flow::dsl::get_edges(input));

        constexpr auto node_capacity = stdx::tuple_size_v<decltype(node_set)>;
        constexpr auto edge_capacity = edge_size(node_set, edges);

        using output_t = Impl<Graph::name, LogPolicy, node_capacity>;
        static_assert(
            all_of(
                []<typename N>(N const &) {
                    return detail::is_output_compatible<N, output_t>;
                },
                node_set),
            "Output node type is not compatible with given input nodes");

        auto g =
            make_graph<output_t, node_capacity, edge_capacity>(node_set, edges);
        return topo_sort<output_t>(g);
    }

    constexpr static auto name = Name;
    using interface_t = auto (*)() -> void;

    template <typename Initialized> class built_flow {
        constexpr static auto built() {
            constexpr auto v = Initialized::value;
            constexpr auto built = build(v);
            static_assert(built.has_value(),
                          "Topological sort failed: cycle in flow");

            using impl_t = typename decltype(built)::value_type;
            constexpr auto nodes = built->nodes;
            return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                return typename impl_t::template finalized_t<nodes[Is]...>{};
            }(std::make_index_sequence<std::size(nodes)>{});
        }

        constexpr static auto run() { built()(); }

      public:
        // NOLINTNEXTLINE(google-explicit-constructor)
        constexpr explicit(false) operator interface_t() const { return run; }
        constexpr auto operator()() const { return run(); }
        constexpr static bool active = decltype(built())::active;
    };

    template <typename Initialized>
    [[nodiscard]] constexpr static auto render() -> built_flow<Initialized> {
        return {};
    }
};
} // namespace flow
