#pragma once

#include <cib/detail/runtime_conditional.hpp>
#include <flow/detail/walk.hpp>
#include <flow/subgraph_identity.hpp>

#include <stdx/tuple_algorithms.hpp>

namespace flow::dsl {
template <subgraph Lhs, subgraph Rhs,
          subgraph_identity Identity = subgraph_identity::REFERENCE,
          typename Cond = cib::detail::always_condition_t>
struct seq {
    Lhs lhs;
    Rhs rhs;

    using is_subgraph = void;

    constexpr auto operator*() const {
        return seq<Lhs, Rhs, subgraph_identity::VALUE, Cond>{Lhs{}, Rhs{}};
    }

  private:
    friend constexpr auto tag_invoke(get_initials_t, seq const &s) {
        return get_initials(s.lhs);
    }

    friend constexpr auto tag_invoke(get_finals_t, seq const &s) {
        return get_finals(s.rhs);
    }

    friend constexpr auto tag_invoke(get_nodes_t, seq const &s) {
        if constexpr (Identity == subgraph_identity::VALUE) {
            auto all_nodes = stdx::to_unsorted_set(
                stdx::tuple_cat(get_all_mentioned_nodes(s.lhs),
                                get_all_mentioned_nodes(s.rhs)));

            return stdx::transform([](auto const &n) { return *n; }, all_nodes);

        } else {
            return stdx::tuple_cat(get_nodes(s.lhs), get_nodes(s.rhs));
        }
    }

    friend constexpr auto tag_invoke(get_all_mentioned_nodes_t, seq const &s) {
        return stdx::tuple_cat(get_all_mentioned_nodes(s.lhs),
                               get_all_mentioned_nodes(s.rhs));
    }

    friend constexpr auto tag_invoke(get_edges_t, seq const &s) {
        auto is = get_initials(s.rhs);
        auto fs = get_finals(s.lhs);

        return stdx::tuple_cat(
            get_edges(s.lhs), get_edges(s.rhs),
            transform(
                []<typename P>(P const &) {
                    return edge<stdx::tuple_element_t<0, P>,
                                stdx::tuple_element_t<1, P>, Cond>{};
                },
                cartesian_product_copy(fs, is)));
    }
};

template <subgraph Lhs, subgraph Rhs> seq(Lhs, Rhs) -> seq<Lhs, Rhs>;

} // namespace flow::dsl

template <flow::dsl::subgraph Lhs, flow::dsl::subgraph Rhs>
[[nodiscard]] constexpr auto operator>>(Lhs const &lhs, Rhs const &rhs) {
    return flow::dsl::seq{lhs, rhs};
}

template <typename Cond, flow::dsl::subgraph Lhs, flow::dsl::subgraph Rhs,
          flow::subgraph_identity Identity, typename EdgeCond>
constexpr auto
make_runtime_conditional(Cond, flow::dsl::seq<Lhs, Rhs, Identity, EdgeCond>) {
    auto lhs = make_runtime_conditional(Cond{}, Lhs{});
    auto rhs = make_runtime_conditional(Cond{}, Rhs{});

    using lhs_t = decltype(lhs);
    using rhs_t = decltype(rhs);
    using cond_t = decltype(EdgeCond{} and Cond{});

    return flow::dsl::seq<lhs_t, rhs_t, Identity, cond_t>{lhs, rhs};
}
