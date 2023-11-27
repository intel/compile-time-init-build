#pragma once

#include <flow/detail/walk.hpp>

#include <stdx/tuple_algorithms.hpp>

namespace flow::dsl {
template <node Lhs, node Rhs> struct seq {
    Lhs lhs;
    Rhs rhs;

    using is_node = void;

  private:
    friend constexpr auto tag_invoke(get_initials_t, seq const &s) {
        return get_initials(s.lhs);
    }

    friend constexpr auto tag_invoke(get_finals_t, seq const &s) {
        return get_finals(s.rhs);
    }

    friend constexpr auto tag_invoke(get_nodes_t, seq const &s) {
        return stdx::tuple_cat(get_nodes(s.lhs), get_nodes(s.rhs));
    }

    friend constexpr auto tag_invoke(get_edges_t, seq const &s) {
        auto is = get_initials(s.rhs);
        auto fs = get_finals(s.lhs);

        return stdx::tuple_cat(
            get_edges(s.lhs), get_edges(s.rhs),
            transform(
                []<typename P>(P const &) {
                    return edge<stdx::tuple_element_t<0, P>,
                                stdx::tuple_element_t<1, P>>{};
                },
                cartesian_product_copy(fs, is)));
    }
};

template <node Lhs, node Rhs> seq(Lhs, Rhs) -> seq<Lhs, Rhs>;
} // namespace flow::dsl

template <flow::dsl::node Lhs, flow::dsl::node Rhs>
[[nodiscard]] constexpr auto operator>>(Lhs const &lhs, Rhs const &rhs) {
    return flow::dsl::seq{lhs, rhs};
}
