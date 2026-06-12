#pragma once

#include <flow/dsl/subgraph_identity.hpp>
#include <flow/dsl/walk.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>

namespace flow::dsl {
template <subgraph Lhs, subgraph Rhs,
          subgraph_identity Identity = subgraph_identity::REFERENCE>
struct par {
    Lhs lhs;
    Rhs rhs;

    using is_subgraph = void;
    using is_composite = void;

    constexpr auto operator*() const {
        return par<Lhs, Rhs, subgraph_identity::VALUE>{Lhs{}, Rhs{}};
    }
};

template <subgraph Lhs, subgraph Rhs> par(Lhs, Rhs) -> par<Lhs, Rhs>;

namespace detail {
template <typename L, typename R, subgraph_identity I>
struct initials_of<par<L, R, I>> {
    using type = boost::mp11::mp_append<initials_of_t<L>, initials_of_t<R>>;
};

template <typename L, typename R, subgraph_identity I>
struct finals_of<par<L, R, I>> {
    using type = boost::mp11::mp_append<finals_of_t<L>, finals_of_t<R>>;
};

template <typename L, typename R, subgraph_identity I>
struct all_mentioned_of<par<L, R, I>> {
    using type =
        boost::mp11::mp_append<all_mentioned_of_t<L>, all_mentioned_of_t<R>>;
};

template <typename L, typename R>
struct nodes_of<par<L, R, subgraph_identity::VALUE>> {
    using type = boost::mp11::mp_transform<
        star_t, boost::mp11::mp_unique<
                    all_mentioned_of_t<par<L, R, subgraph_identity::VALUE>>>>;
};

template <typename L, typename R>
struct nodes_of<par<L, R, subgraph_identity::REFERENCE>> {
    using type = boost::mp11::mp_append<nodes_of_t<L>, nodes_of_t<R>>;
};

template <typename L, typename R, subgraph_identity I>
struct edges_of<par<L, R, I>> {
    using type = boost::mp11::mp_append<edges_of_t<L>, edges_of_t<R>>;
};
} // namespace detail

} // namespace flow::dsl

template <flow::dsl::subgraph Lhs, flow::dsl::subgraph Rhs>
[[nodiscard]] constexpr auto operator&&(Lhs const &lhs, Rhs const &rhs) {
    return flow::dsl::par{lhs, rhs};
}

template <typename Cond, flow::dsl::subgraph Lhs, flow::dsl::subgraph Rhs,
          flow::dsl::subgraph_identity Identity>
constexpr auto make_runtime_conditional(Cond,
                                        flow::dsl::par<Lhs, Rhs, Identity>) {
    auto lhs = make_runtime_conditional(Cond{}, Lhs{});
    auto rhs = make_runtime_conditional(Cond{}, Rhs{});

    using lhs_t = decltype(lhs);
    using rhs_t = decltype(rhs);

    return flow::dsl::par<lhs_t, rhs_t, Identity>{lhs, rhs};
}
