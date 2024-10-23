#pragma once

#include <flow/common.hpp>
#include <flow/detail/walk.hpp>
#include <flow/impl.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/cx_multimap.hpp>
#include <stdx/cx_set.hpp>
#include <stdx/cx_vector.hpp>
#include <stdx/span.hpp>
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
}
template <typename T> using name_for = typename T::name_t;

[[nodiscard]] constexpr auto edge_size(auto const &nodes,
                                       auto const &edges) -> std::size_t {
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

template <typename Cond> struct sequence_conditions;

template <auto...> struct INFO_flow_name_;

template <auto...> struct INFO_step_a_name_;

template <auto...> struct INFO_step_a_condition_;

template <auto...> struct INFO_step_b_name_;

template <auto...> struct INFO_step_b_condition_;

template <auto...> struct INFO_sequence_condition_;

template <typename...> struct INFO_sequence_missing_predicate_;

template <typename...> constexpr bool ERROR_DETAILS_ = false;

template <auto...> struct INFO_missing_steps_;

template <auto...> struct INFO_duplicated_steps_;

template <stdx::ct_string Name,
          template <stdx::ct_string, std::size_t> typename Impl>
struct graph_builder {
    template <typename Node, std::size_t N, std::size_t E>
    [[nodiscard]] constexpr static auto make_graph(auto const &nodes,
                                                   auto const &edges) {
        using graph_t = stdx::cx_multimap<Node, Node, N, E>;
        graph_t g{};
        for_each([&](auto const &node) { g.put(node); }, nodes);

        auto const named_nodes = stdx::apply_indices<name_for>(nodes);
        for_each(
            [&]<typename Lhs, typename Rhs, typename Cond>(
                dsl::edge<Lhs, Rhs, Cond> const &) {
                auto lhs = get<name_for<Lhs>>(named_nodes);
                auto rhs = get<name_for<Rhs>>(named_nodes);

                using lhs_t = std::remove_cvref_t<decltype(lhs)>;
                using rhs_t = std::remove_cvref_t<decltype(rhs)>;

                using edge_ps_t = decltype(Cond::predicates);
                auto node_ps = stdx::to_unsorted_set(stdx::tuple_cat(
                    lhs_t::condition.predicates, rhs_t::condition.predicates));

                stdx::for_each(
                    [&]<typename P>(P const &) {
                        if constexpr (not stdx::contains_type<edge_ps_t, P>) {
                            static_assert(
                                ERROR_DETAILS_<
                                    INFO_flow_name_<Name>,
                                    INFO_step_a_name_<lhs_t::ct_name>,
                                    INFO_step_a_condition_<
                                        lhs_t::condition.ct_name>,
                                    INFO_step_b_name_<rhs_t::ct_name>,
                                    INFO_step_b_condition_<
                                        rhs_t::condition.ct_name>,
                                    INFO_sequence_condition_<Cond::ct_name>,
                                    INFO_sequence_missing_predicate_<P>>,

                                "The conditions on this sequence "
                                "(step_a >> step_b) are weaker than those on "
                                "step_a and/or step_b. The sequence could be "
                                "enabled while step_a and/or step_b is not. "
                                "Specifically, the sequence is missing the "
                                "predicate identified in "
                                "`INFO_sequence_missing_predicate_`. TIP: "
                                "Look for 'ERROR_DETAILS_` and `INFO_` in "
                                "the compiler error message for details on "
                                "the sequence, the step names, and the "
                                "conditions.");
                        }
                    },
                    node_ps);

                g.put(lhs, rhs);
            },
            edges);
        return g;
    }

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
    [[nodiscard]] constexpr static auto
    topo_sort(Graph &g) -> std::optional<Output> {
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

    constexpr static void check_for_missing_nodes(auto nodes,
                                                  auto mentioned_nodes) {
        constexpr auto get_name = []<typename N>(N) ->
            typename N::name_t { return {}; };
        auto node_names = stdx::transform(get_name, nodes);
        auto mentioned_node_names = stdx::transform(get_name, mentioned_nodes);

        using node_names_t = decltype(stdx::to_sorted_set(node_names));
        using mentioned_node_names_t =
            decltype(stdx::to_sorted_set(mentioned_node_names));

        if constexpr (not std::is_same_v<node_names_t,
                                         mentioned_node_names_t>) {
            using missing_nodes_t =
                boost::mp11::mp_set_difference<mentioned_node_names_t,
                                               node_names_t>;

            constexpr auto missing_nodes = missing_nodes_t{};
            constexpr auto missing_nodes_ct_strings = stdx::transform(
                []<typename N>(N) { return stdx::ct_string_from_type(N{}); },
                missing_nodes);

            [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                using error_details_t = INFO_missing_steps_<stdx::get<Is>(
                    missing_nodes_ct_strings)...>;

                static_assert(
                    ERROR_DETAILS_<INFO_flow_name_<Name>, error_details_t>,
                    "One or more steps are referenced in the flow but not "
                    "explicitly added with the * operator. The "
                    "beginning of this error shows you which steps are "
                    "missing.");
            }(std::make_index_sequence<
                stdx::tuple_size_v<decltype(missing_nodes)>>{});
        }

        if constexpr (stdx::tuple_size_v<node_names_t> !=
                      stdx::tuple_size_v<decltype(node_names)>) {
            constexpr auto duplicates = stdx::transform(
                [](auto e) {
                    return stdx::ct_string_from_type(stdx::get<0>(e));
                },
                stdx::filter<detail::is_duplicated>(stdx::gather(node_names)));

            [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                using error_details_t =
                    INFO_duplicated_steps_<stdx::get<Is>(duplicates)...>;

                static_assert(
                    ERROR_DETAILS_<INFO_flow_name_<Name>, error_details_t>,
                    "One or more steps are explicitly added more than once "
                    "using the * operator. The beginning of this "
                    "error shows you which steps are duplicated.");
            }(std::make_index_sequence<
                stdx::tuple_size_v<decltype(duplicates)>>{});
        }
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

        using output_t = Impl<Graph::name, node_capacity>;
        using rt_node_t = typename output_t::node_t;

        static_assert(
            all_of(
                []<typename N>(N const &) {
                    return std::is_convertible_v<N, rt_node_t>;
                },
                node_set),
            "Output node type is not compatible with given input nodes");

        auto g = make_graph<rt_node_t, node_capacity, edge_capacity>(node_set,
                                                                     edges);
        return topo_sort<output_t>(g);
    }

    template <typename Initialized> class built_flow {
        constexpr static auto built() {
            constexpr auto v = Initialized::value;
            constexpr auto built = build(v);
            static_assert(built.has_value(),
                          "Topological sort failed: cycle in flow");

            constexpr auto functionPtrs = built->functionPtrs;
            constexpr auto size = functionPtrs.size();
            constexpr auto name = built->name;

            return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                return detail::inlined_func_list<name, functionPtrs[Is]...>{};
            }(std::make_index_sequence<size>{});
        }

        constexpr static auto run() { built()(); }

      public:
        // NOLINTNEXTLINE(google-explicit-constructor)
        constexpr explicit(false) operator FunctionPtr() const { return run; }
        constexpr auto operator()() const -> void { run(); }
        constexpr static bool active = decltype(built())::active;
    };

    template <typename Initialized>
    [[nodiscard]] constexpr static auto render() -> built_flow<Initialized> {
        return {};
    }
};

template <stdx::ct_string Name = "",
          typename Renderer = graph_builder<Name, impl>,
          flow::dsl::subgraph... Fragments>
class graph {
    template <typename Tag>
    friend constexpr auto tag_invoke(Tag, graph const &g) {
        return g.fragments.apply([](auto const &...frags) {
            return stdx::tuple_cat(Tag{}(frags)...);
        });
    }

  public:
    template <flow::dsl::subgraph... Ns>
    [[nodiscard]] constexpr auto add(Ns &&...ns) {
        return fragments.apply([&](auto &...frags) {
            return graph<Name, Renderer, Fragments...,
                         stdx::remove_cvref_t<Ns>...>{
                {frags..., std::forward<Ns>(ns)...}};
        });
    }

    template <typename BuilderValue>
    [[nodiscard]] constexpr static auto build() {
        return Renderer::template render<BuilderValue>();
    }

    constexpr static auto name = Name;
    stdx::tuple<Fragments...> fragments;
};
} // namespace flow
