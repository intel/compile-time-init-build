#pragma once

#include <flow/dsl/subgraph_identity.hpp>

#include <stdx/concepts.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace flow::dsl {
template <typename T>
concept subgraph = requires { typename stdx::remove_cvref_t<T>::is_subgraph; };

template <typename Source, typename Dest, typename Cond> struct edge {
    using source_t = Source;
    using dest_t = Dest;
    using cond_t = Cond;
};

template <stdx::ct_string Name> struct node_reference {
    using is_subgraph = void;
    constexpr static auto identity = subgraph_identity::REFERENCE;
    using name_t = stdx::cts_t<Name>;
};

namespace literals {
template <stdx::ct_string S> [[nodiscard]] constexpr auto operator""_ref() {
    return node_reference<S>{};
}
} // namespace literals
} // namespace flow::dsl

namespace flow::dsl::detail {

template <typename E> struct initials_of {
    using type = boost::mp11::mp_list<E>;
};

template <typename... T> using initials_of_t = typename initials_of<T...>::type;

template <typename E> struct finals_of {
    using type = boost::mp11::mp_list<E>;
};

template <typename... T> using finals_of_t = typename finals_of<T...>::type;

template <typename E> struct all_mentioned_of {
    using type = boost::mp11::mp_list<E>;
};

template <typename... T>
using all_mentioned_of_t = typename all_mentioned_of<T...>::type;

template <typename E> struct nodes_of {
    using type =
        std::conditional_t<E::identity == subgraph_identity::VALUE,
                           boost::mp11::mp_list<E>, boost::mp11::mp_list<>>;
};

template <typename... T> using nodes_of_t = typename nodes_of<T...>::type;

template <typename E> struct edges_of {
    using type = boost::mp11::mp_list<>;
};

template <typename... T> using edges_of_t = typename edges_of<T...>::type;

template <typename N>
using star_t = std::remove_cvref_t<decltype(*std::declval<N>())>;

template <typename Cond> struct make_edge_q {
    template <typename A, typename B> using fn = edge<A, B, Cond>;
};

template <typename List>
constexpr auto to_tuple_v = boost::mp11::mp_apply<stdx::tuple, List>{};

template <typename T>
concept builder_like = requires { typename T::is_builder; };

template <typename T>
concept composite_like = requires { typename T::is_composite; };

template <typename N> constexpr auto collect_node_values(N const &);

[[nodiscard]] constexpr auto collect_node_values(builder_like auto const &b) {
    return b.fragments.apply([](auto const &...fs) {
        return stdx::tuple_cat(collect_node_values(fs)...);
    });
}

[[nodiscard]] constexpr auto collect_node_values(composite_like auto const &c) {
    return stdx::tuple_cat(collect_node_values(c.lhs),
                           collect_node_values(c.rhs));
}

template <typename Leaf>
[[nodiscard]] constexpr auto collect_node_values(Leaf const &l) {
    return stdx::make_tuple(l);
}

// Find the node in the existing leaf instantiations
template <typename N, typename Leaves>
[[nodiscard]] constexpr auto pick_node(Leaves const &leaves) {
    using leaves_t = stdx::remove_cvref_t<Leaves>;
    using leaf_instance_list =
        boost::mp11::mp_apply<boost::mp11::mp_list,
                              boost::mp11::mp_transform<star_t, leaves_t>>;
    constexpr auto idx = boost::mp11::mp_find<leaf_instance_list, N>::value;
    if constexpr (idx < stdx::tuple_size_v<leaves_t>) {
        return *stdx::get<idx>(leaves);
    } else {
        return N{};
    }
}

template <typename... Ns, typename Leaves>
[[nodiscard]] constexpr auto pick_nodes(boost::mp11::mp_list<Ns...>,
                                        Leaves const &leaves) {
    return stdx::make_tuple(pick_node<Ns>(leaves)...);
}
} // namespace flow::dsl::detail

namespace flow::dsl {
template <typename T> [[nodiscard]] constexpr auto get_nodes(T const &t) {
    return detail::pick_nodes(detail::nodes_of_t<stdx::remove_cvref_t<T>>{},
                              detail::collect_node_values(t));
}

template <typename T>
[[nodiscard]] constexpr auto get_all_mentioned_nodes(T const &) {
    return detail::to_tuple_v<
        detail::all_mentioned_of_t<stdx::remove_cvref_t<T>>>;
}

template <typename T> [[nodiscard]] constexpr auto get_edges(T const &) {
    return detail::to_tuple_v<detail::edges_of_t<stdx::remove_cvref_t<T>>>;
}
} // namespace flow::dsl
