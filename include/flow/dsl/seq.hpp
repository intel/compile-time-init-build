#pragma once

#include <flow/dsl/flatten.hpp>
#include <flow/dsl/subgraph_identity.hpp>
#include <flow/dsl/walk.hpp>
#include <nexus/detail/runtime_conditional.hpp>

#include <stdx/tuple_algorithms.hpp>

#include <boost/mp11.hpp>

namespace flow::dsl {
template <subgraph Lhs, subgraph Rhs,
          subgraph_identity Identity = subgraph_identity::REFERENCE,
          typename Cond = cib::detail::always_condition_t>
struct seq {
    [[no_unique_address]] Lhs lhs;
    [[no_unique_address]] Rhs rhs;

    using is_subgraph = void;

    constexpr auto operator*() const {
        return seq<Lhs, Rhs, subgraph_identity::VALUE, Cond>{Lhs{}, Rhs{}};
    }

  private:
    friend constexpr auto tag_invoke(get_initials_t, seq const &) {
        return detail::to_tuple_v<typename detail::initials_of<seq>::type>;
    }

    friend constexpr auto tag_invoke(get_finals_t, seq const &) {
        return detail::to_tuple_v<typename detail::finals_of<seq>::type>;
    }

    friend constexpr auto tag_invoke(get_nodes_t, seq const &) {
        return detail::to_tuple_v<typename detail::nodes_of<seq>::type>;
    }

    friend constexpr auto tag_invoke(get_all_mentioned_nodes_t, seq const &) {
        return detail::to_tuple_v<typename detail::all_mentioned_of<seq>::type>;
    }

    friend constexpr auto tag_invoke(get_edges_t, seq const &) {
        return detail::to_tuple_v<typename detail::edges_of<seq>::type>;
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
    using type = boost::mp11::mp_append<typename all_mentioned_of<L>::type,
                                        typename all_mentioned_of<R>::type>;
};

template <typename L, typename R, subgraph_identity I, typename C>
struct nodes_of<seq<L, R, I, C>> {
    using value_nodes = boost::mp11::mp_transform<
        star_t, boost::mp11::mp_unique<
                    typename all_mentioned_of<seq<L, R, I, C>>::type>>;
    using ref_nodes = boost::mp11::mp_append<typename nodes_of<L>::type,
                                             typename nodes_of<R>::type>;
    using type = std::conditional_t<I == subgraph_identity::VALUE, value_nodes,
                                    ref_nodes>;
};

template <typename L, typename R, subgraph_identity I, typename C>
struct edges_of<seq<L, R, I, C>> {
    using pairs =
        boost::mp11::mp_product<boost::mp11::mp_list,
                                typename finals_of<L>::type,
                                typename initials_of<R>::type>;
    using cross = boost::mp11::mp_transform_q<make_edge_q<C>, pairs>;
    using type = boost::mp11::mp_append<typename edges_of<L>::type,
                                        typename edges_of<R>::type, cross>;
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
