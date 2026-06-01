#pragma once

#include <flow/dsl/flatten.hpp>
#include <flow/dsl/subgraph_identity.hpp>
#include <flow/dsl/walk.hpp>

#include <stdx/tuple_algorithms.hpp>

#include <boost/mp11.hpp>

namespace flow::dsl {
template <subgraph Lhs, subgraph Rhs,
          subgraph_identity Identity = subgraph_identity::REFERENCE>
struct par {
    [[no_unique_address]] Lhs lhs;
    [[no_unique_address]] Rhs rhs;

    using is_subgraph = void;

    constexpr auto operator*() const {
        return par<Lhs, Rhs, subgraph_identity::VALUE>{Lhs{}, Rhs{}};
    }

  private:
    friend constexpr auto tag_invoke(get_initials_t, par const &) {
        return detail::to_tuple_v<typename detail::initials_of<par>::type>;
    }

    friend constexpr auto tag_invoke(get_finals_t, par const &) {
        return detail::to_tuple_v<typename detail::finals_of<par>::type>;
    }

    friend constexpr auto tag_invoke(get_nodes_t, par const &) {
        return detail::to_tuple_v<typename detail::nodes_of<par>::type>;
    }

    friend constexpr auto tag_invoke(get_all_mentioned_nodes_t, par const &) {
        return detail::to_tuple_v<typename detail::all_mentioned_of<par>::type>;
    }

    friend constexpr auto tag_invoke(get_edges_t, par const &) {
        return detail::to_tuple_v<typename detail::edges_of<par>::type>;
    }
};

template <subgraph Lhs, subgraph Rhs> par(Lhs, Rhs) -> par<Lhs, Rhs>;

namespace detail {
template <typename L, typename R, subgraph_identity I>
struct initials_of<par<L, R, I>> {
    using type = boost::mp11::mp_append<typename initials_of<L>::type,
                                        typename initials_of<R>::type>;
};

template <typename L, typename R, subgraph_identity I>
struct finals_of<par<L, R, I>> {
    using type = boost::mp11::mp_append<typename finals_of<L>::type,
                                        typename finals_of<R>::type>;
};

template <typename L, typename R, subgraph_identity I>
struct all_mentioned_of<par<L, R, I>> {
    using type = boost::mp11::mp_append<typename all_mentioned_of<L>::type,
                                        typename all_mentioned_of<R>::type>;
};

template <typename L, typename R, subgraph_identity I>
struct nodes_of<par<L, R, I>> {
    using value_nodes = boost::mp11::mp_transform<
        star_t,
        boost::mp11::mp_unique<typename all_mentioned_of<par<L, R, I>>::type>>;
    using ref_nodes = boost::mp11::mp_append<typename nodes_of<L>::type,
                                             typename nodes_of<R>::type>;
    using type = std::conditional_t<I == subgraph_identity::VALUE, value_nodes,
                                    ref_nodes>;
};

template <typename L, typename R, subgraph_identity I>
struct edges_of<par<L, R, I>> {
    using type = boost::mp11::mp_append<typename edges_of<L>::type,
                                        typename edges_of<R>::type>;
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
