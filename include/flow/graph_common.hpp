#pragma once

#include <stdx/ct_string.hpp>

#include <boost/mp11/algorithm.hpp>

#include <algorithm>
#include <cstddef>
#include <type_traits>

namespace flow {
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

namespace detail {
template <typename N> struct matching_source {
    template <typename Edge>
    using fn = std::is_same<name_for<N>, name_for<typename Edge::source_t>>;
};

template <typename N> struct matching_dest {
    template <typename Edge>
    using fn = std::is_same<name_for<N>, name_for<typename Edge::dest_t>>;
};

template <typename T> struct matching_name {
    template <typename U> using fn = std::is_same<name_for<T>, name_for<U>>;
};

template <typename Nodes> struct contained_edge_q {
    template <typename Edge>
    using fn = std::bool_constant<
        not boost::mp11::mp_empty<boost::mp11::mp_copy_if_q<
            Nodes, matching_name<typename Edge::source_t>>>::value and
        not boost::mp11::mp_empty<boost::mp11::mp_copy_if_q<
            Nodes, matching_name<typename Edge::dest_t>>>::value>;
};

template <template <typename> typename Matcher, typename Edges>
struct has_no_edge_q {
    template <typename Node>
    using fn =
        boost::mp11::mp_empty<boost::mp11::mp_copy_if_q<Edges, Matcher<Node>>>;
};

template <typename E> using parent_of = typename E::source_t;
template <typename E> using child_of = typename E::dest_t;

template <typename Node, typename Edges>
using parents_of = boost::mp11::mp_transform<
    parent_of, boost::mp11::mp_copy_if_q<Edges, matching_dest<Node>>>;

template <typename Node, typename Edges>
using children_of = boost::mp11::mp_transform<
    child_of, boost::mp11::mp_copy_if_q<Edges, matching_source<Node>>>;

template <typename...> struct descendant_of_q;

template <typename L, typename Start, typename Edges>
using any_descendants =
    boost::mp11::mp_any_of_q<L, descendant_of_q<Start, Edges>>;

template <typename Start, typename Edges> struct descendant_of_q<Start, Edges> {
    template <typename Node>
    using fn = boost::mp11::mp_eval_if_c<
        std::is_same_v<name_for<Node>, name_for<Start>>, std::true_type,
        any_descendants, parents_of<Node, Edges>, Start, Edges>;
};

template <typename...> struct ancestor_of_q;

template <typename L, typename Start, typename Edges>
using any_ancestors = boost::mp11::mp_any_of_q<L, ancestor_of_q<Start, Edges>>;

template <typename Start, typename Edges> struct ancestor_of_q<Start, Edges> {
    template <typename Node>
    using fn = boost::mp11::mp_eval_if_c<
        std::is_same_v<name_for<Node>, name_for<Start>>, std::true_type,
        any_ancestors, children_of<Node, Edges>, Start, Edges>;
};
} // namespace detail
} // namespace flow
