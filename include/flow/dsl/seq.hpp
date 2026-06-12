#pragma once

#include <flow/dsl/subgraph_identity.hpp>
#include <flow/dsl/walk.hpp>
#include <nexus/detail/runtime_conditional.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>

namespace flow::dsl {
template <subgraph Lhs, subgraph Rhs,
          subgraph_identity Identity = subgraph_identity::REFERENCE,
          typename Cond = cib::detail::always_condition_t>
struct seq {
    Lhs lhs;
    Rhs rhs;

    using is_subgraph = void;
    using is_composite = void;

    constexpr auto operator*() const {
        return seq<Lhs, Rhs, subgraph_identity::VALUE, Cond>{Lhs{}, Rhs{}};
    }
};

template <subgraph Lhs, subgraph Rhs> seq(Lhs, Rhs) -> seq<Lhs, Rhs>;

namespace detail {
template <typename L, typename R, subgraph_identity I, typename C>
struct initials_of<seq<L, R, I, C>> {
    using type = typename initials_of<L>::type;
};

template <typename L, typename R, subgraph_identity I, typename C>
struct finals_of<seq<L, R, I, C>> {
    using type = typename finals_of<R>::type;
};

template <typename L, typename R, subgraph_identity I, typename C>
struct all_mentioned_of<seq<L, R, I, C>> {
    using type =
        boost::mp11::mp_append<all_mentioned_of_t<L>, all_mentioned_of_t<R>>;
};

template <typename L, typename R, typename C>
struct nodes_of<seq<L, R, subgraph_identity::VALUE, C>> {
    using type =
        boost::mp11::mp_transform<star_t,
                                  boost::mp11::mp_unique<all_mentioned_of_t<
                                      seq<L, R, subgraph_identity::VALUE, C>>>>;
};

template <typename L, typename R, typename C>
struct nodes_of<seq<L, R, subgraph_identity::REFERENCE, C>> {
    using type = boost::mp11::mp_append<nodes_of_t<L>, nodes_of_t<R>>;
};

template <typename L, typename R, subgraph_identity I, typename C>
struct edges_of<seq<L, R, I, C>> {
    using cross = boost::mp11::mp_product_q<make_edge_q<C>, finals_of_t<L>,
                                            initials_of_t<R>>;
    using type = boost::mp11::mp_append<edges_of_t<L>, edges_of_t<R>, cross>;
};
} // namespace detail

} // namespace flow::dsl

template <flow::dsl::subgraph Lhs, flow::dsl::subgraph Rhs>
[[nodiscard]] constexpr auto operator>>(Lhs const &lhs, Rhs const &rhs) {
    return flow::dsl::seq{lhs, rhs};
}

template <typename Cond, flow::dsl::subgraph Lhs, flow::dsl::subgraph Rhs,
          flow::dsl::subgraph_identity Identity, typename EdgeCond>
constexpr auto
make_runtime_conditional(Cond, flow::dsl::seq<Lhs, Rhs, Identity, EdgeCond>) {
    auto lhs = make_runtime_conditional(Cond{}, Lhs{});
    auto rhs = make_runtime_conditional(Cond{}, Rhs{});

    using lhs_t = decltype(lhs);
    using rhs_t = decltype(rhs);
    using cond_t = decltype(EdgeCond{} and Cond{});

    return flow::dsl::seq<lhs_t, rhs_t, Identity, cond_t>{lhs, rhs};
}
