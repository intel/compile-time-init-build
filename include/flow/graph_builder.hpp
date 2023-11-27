#pragma once

#include <flow/common.hpp>
#include <flow/detail/walk.hpp>
#include <flow/milestone.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/cx_multimap.hpp>
#include <stdx/cx_set.hpp>
#include <stdx/cx_vector.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <optional>
#include <span>
#include <utility>

namespace flow {
template <stdx::ct_string Name,
          template <stdx::ct_string, std::size_t> typename Impl,
          flow::dsl::node... Fragments>
class graph_builder {
    friend constexpr auto tag_invoke(flow::dsl::get_nodes_t,
                                     graph_builder const &b) {
        auto t = b.fragments.apply([](auto const &...frags) {
            return stdx::tuple_cat(flow::dsl::get_nodes(frags)...);
        });
        return stdx::to_unsorted_set(t);
    }

    friend constexpr auto tag_invoke(flow::dsl::get_edges_t,
                                     graph_builder const &b) {
        auto t = b.fragments.apply([](auto const &...frags) {
            return stdx::tuple_cat(flow::dsl::get_edges(frags)...);
        });
        return stdx::to_unsorted_set(t);
    }

    [[nodiscard]] constexpr static auto edge_size(auto const &nodes,
                                                  auto const &edges)
        -> std::size_t {
        auto const edge_capacities = transform(
            [&]<typename N>(N const &) {
                return edges.fold_left(
                    std::size_t{}, []<typename E>(auto acc, E const &) {
                        if constexpr (std::is_same_v<typename E::source_t, N> or
                                      std::is_same_v<typename E::dest_t, N>) {
                            return ++acc;
                        } else {
                            return acc;
                        }
                    });
            },
            nodes);

        return edge_capacities.fold_left(
            std::size_t{1},
            [](auto acc, auto next) { return std::max(acc, next); });
    }

    template <typename T> using name_for = typename T::name_t;

    template <typename Node, std::size_t N, std::size_t E>
    [[nodiscard]] constexpr static auto make_graph(auto const &nodes,
                                                   auto const &edges) {
        using graph_t = stdx::cx_multimap<Node, Node, N, E>;
        graph_t g{};
        for_each([&](auto const &node) { g.put(node); }, nodes);

        auto const named_nodes = stdx::apply_indices<name_for>(nodes);
        for_each(
            [&]<typename Edge>(Edge const &) {
                g.put(get<name_for<typename Edge::source_t>>(named_nodes),
                      get<name_for<typename Edge::dest_t>>(named_nodes));
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
    [[nodiscard]] constexpr auto topo_sort(Graph &g) const
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
        return std::optional<Output>{
            std::in_place,
            std::span{std::cbegin(ordered_list), std::size(ordered_list)}};
    }

    template <typename BuilderValue> class built_flow {
        constexpr static auto built() {
            constexpr auto builder = BuilderValue::value;
            constexpr auto built = builder.template topo_sort<Impl>();
            static_assert(built.has_value());
            return *built;
        }

        constexpr static auto run() { built()(); }

      public:
        // NOLINTNEXTLINE(google-explicit-constructor)
        constexpr operator FunctionPtr() const { return run; }
        constexpr auto operator()() const -> void { run(); }
        constexpr static bool active = decltype(built())::active;
    };

  public:
    constexpr static auto name = Name;

    template <flow::dsl::node... Ns>
    [[nodiscard]] constexpr auto add(Ns &&...ns) {
        return fragments.apply([&](auto &...frags) {
            return graph_builder<Name, Impl, Fragments...,
                                 stdx::remove_cvref_t<Ns>...>{
                {frags..., std::forward<Ns>(ns)...}};
        });
    }

    template <template <stdx::ct_string, std::size_t> typename Output>
    [[nodiscard]] constexpr auto topo_sort() const {
        auto nodes = flow::dsl::get_nodes(*this);
        auto edges = flow::dsl::get_edges(*this);

        constexpr auto node_capacity = stdx::tuple_size_v<decltype(nodes)>;
        constexpr auto edge_capacity = edge_size(nodes, edges);

        using output_t = Output<Name, node_capacity>;
        using rt_node_t = typename output_t::node_t;

        static_assert(
            all_of(
                []<typename N>(N const &) {
                    return std::is_convertible_v<N, rt_node_t>;
                },
                nodes),
            "Output node type is not compatible with given input nodes");

        auto g =
            make_graph<rt_node_t, node_capacity, edge_capacity>(nodes, edges);
        return topo_sort<output_t>(g);
    }

    [[nodiscard]] constexpr auto node_size() const -> std::size_t {
        return stdx::tuple_size_v<decltype(flow::dsl::get_nodes(*this))>;
    }

    [[nodiscard]] constexpr auto edge_size() const -> std::size_t {
        return edge_size(flow::dsl::get_nodes(*this),
                         flow::dsl::get_edges(*this));
    }

    template <typename BuilderValue>
    [[nodiscard]] constexpr static auto build() -> built_flow<BuilderValue> {
        return {};
    }

    stdx::tuple<Fragments...> fragments;
};
} // namespace flow
