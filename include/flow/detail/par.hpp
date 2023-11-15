#pragma once

#include <flow/detail/walk.hpp>

#include <array>
#include <cstddef>
#include <utility>

namespace flow::dsl {
template <typename T, std::size_t N, std::size_t M>
constexpr auto concat(std::array<T, N> a, std::array<T, M> b)
    -> std::array<T, N + M> {
    return [&]<std::size_t... Is, std::size_t... Js>(
               std::index_sequence<Is...>, std::index_sequence<Js...>) {
        return std::array{a[Is]..., b[Js]...};
    }(std::make_index_sequence<N>{}, std::make_index_sequence<M>{});
}

template <node Lhs, node Rhs> struct par {
    Lhs lhs;
    Rhs rhs;

    using is_node = void;

  private:
    template <typename F>
    friend constexpr auto tag_invoke(walk_t, F &&f, par const &p) -> void {
        walk(f, p.lhs);
        walk(f, p.rhs);
    }

    friend constexpr auto tag_invoke(get_initials_t, par const &p) {
        return concat(get_initials(p.lhs), get_initials(p.rhs));
    }

    friend constexpr auto tag_invoke(get_finals_t, par const &p) {
        return concat(get_finals(p.lhs), get_finals(p.rhs));
    }
};

template <node Lhs, node Rhs> par(Lhs, Rhs) -> par<Lhs, Rhs>;
} // namespace flow::dsl

template <flow::dsl::node Lhs, flow::dsl::node Rhs>
[[nodiscard]] constexpr auto operator&&(Lhs const &lhs, Rhs const &rhs) {
    return flow::dsl::par{lhs, rhs};
}
