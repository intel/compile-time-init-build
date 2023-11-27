#pragma once

#include <flow/detail/walk.hpp>

#include <stdx/tuple_algorithms.hpp>

namespace flow::dsl {
template <node Lhs, node Rhs> struct par {
    Lhs lhs;
    Rhs rhs;

    using is_node = void;

  private:
    friend constexpr auto tag_invoke(get_initials_t, par const &p) {
        return stdx::tuple_cat(get_initials(p.lhs), get_initials(p.rhs));
    }

    friend constexpr auto tag_invoke(get_finals_t, par const &p) {
        return stdx::tuple_cat(get_finals(p.lhs), get_finals(p.rhs));
    }

    friend constexpr auto tag_invoke(get_nodes_t, par const &p) {
        return stdx::tuple_cat(get_nodes(p.lhs), get_nodes(p.rhs));
    }

    friend constexpr auto tag_invoke(get_edges_t, par const &p) {
        return stdx::tuple_cat(get_edges(p.lhs), get_edges(p.rhs));
    }
};

template <node Lhs, node Rhs> par(Lhs, Rhs) -> par<Lhs, Rhs>;
} // namespace flow::dsl

template <flow::dsl::node Lhs, flow::dsl::node Rhs>
[[nodiscard]] constexpr auto operator&&(Lhs const &lhs, Rhs const &rhs) {
    return flow::dsl::par{lhs, rhs};
}
