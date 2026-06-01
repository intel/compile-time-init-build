#pragma once

#include <flow/dsl/subgraph_identity.hpp>
#include <flow/dsl/walk.hpp>

#include <stdx/tuple.hpp>

#include <boost/mp11.hpp>

#include <type_traits>

namespace flow::dsl::detail {

template <typename E> struct initials_of {
    using type = boost::mp11::mp_list<E>;
};

template <typename E> struct finals_of {
    using type = boost::mp11::mp_list<E>;
};

template <typename E> struct all_mentioned_of {
    using type = boost::mp11::mp_list<E>;
};

template <typename E> struct nodes_of {
    using type =
        std::conditional_t<E::identity == subgraph_identity::VALUE,
                           boost::mp11::mp_list<E>, boost::mp11::mp_list<>>;
};

template <typename E> struct edges_of {
    using type = boost::mp11::mp_list<>;
};

template <typename N>
using star_t = std::remove_cvref_t<decltype(*std::declval<N>())>;

template <typename Cond> struct make_edge_q {
    template <typename Pair>
    using fn = edge<boost::mp11::mp_first<Pair>, boost::mp11::mp_second<Pair>,
                    Cond>;
};

template <typename List> struct to_tuple;
template <typename... Ts> struct to_tuple<boost::mp11::mp_list<Ts...>> {
    constexpr static auto value = stdx::tuple<Ts...>{};
};
template <typename List> constexpr auto to_tuple_v = to_tuple<List>::value;

} // namespace flow::dsl::detail
