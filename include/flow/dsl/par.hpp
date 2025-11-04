#pragma once

#include <flow/dsl/subgraph_identity.hpp>
#include <flow/dsl/walk.hpp>

#include <stdx/tuple_algorithms.hpp>

namespace flow::dsl {
template <subgraph Lhs, subgraph Rhs,
          subgraph_identity Identity = subgraph_identity::REFERENCE>
struct par {
    Lhs lhs;
    Rhs rhs;

    using is_subgraph = void;

    constexpr auto operator*() const {
        return par<Lhs, Rhs, subgraph_identity::VALUE>{Lhs{}, Rhs{}};
    }

  private:
    friend constexpr auto tag_invoke(get_initials_t, par const &p) {
        return stdx::tuple_cat(get_initials(p.lhs), get_initials(p.rhs));
    }

    friend constexpr auto tag_invoke(get_finals_t, par const &p) {
        return stdx::tuple_cat(get_finals(p.lhs), get_finals(p.rhs));
    }

    friend constexpr auto tag_invoke(get_nodes_t, par const &p) {
        if constexpr (Identity == subgraph_identity::VALUE) {
            auto all_nodes = stdx::to_unsorted_set(
                stdx::tuple_cat(get_all_mentioned_nodes(p.lhs),
                                get_all_mentioned_nodes(p.rhs)));

            return stdx::transform([](auto const &n) { return *n; }, all_nodes);

        } else {
            return stdx::tuple_cat(get_nodes(p.lhs), get_nodes(p.rhs));
        }
    }

    friend constexpr auto tag_invoke(get_all_mentioned_nodes_t, par const &p) {
        return stdx::tuple_cat(get_all_mentioned_nodes(p.lhs),
                               get_all_mentioned_nodes(p.rhs));
    }

    friend constexpr auto tag_invoke(get_edges_t, par const &p) {
        return stdx::tuple_cat(get_edges(p.lhs), get_edges(p.rhs));
    }
};

template <subgraph Lhs, subgraph Rhs> par(Lhs, Rhs) -> par<Lhs, Rhs>;
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
