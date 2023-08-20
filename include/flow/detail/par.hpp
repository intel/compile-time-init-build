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

    constexpr auto walk(auto c) const -> void {
        dsl::walk(c, lhs);
        dsl::walk(c, rhs);
    }

    constexpr auto initials() const {
        return concat(dsl::initials(lhs), dsl::initials(rhs));
    }

    constexpr auto finals() const {
        return concat(dsl::finals(lhs), dsl::finals(rhs));
    }
};

template <node Lhs, node Rhs> par(Lhs, Rhs) -> par<Lhs, Rhs>;
} // namespace flow::dsl

template <flow::dsl::node Lhs, flow::dsl::node Rhs>
[[nodiscard]] constexpr auto operator&&(Lhs const &lhs, Rhs const &rhs) {
    return flow::dsl::par{lhs, rhs};
}
