#pragma once

#include <flow/detail/walk.hpp>

namespace flow::dsl {
template <node Lhs, node Rhs> struct seq {
    Lhs lhs;
    Rhs rhs;

    using is_node = void;

    constexpr auto walk(auto c) const -> void {
        dsl::walk(c, lhs);
        dsl::walk(c, rhs);

        for (auto final : dsl::finals(lhs)) {
            for (auto initial : dsl::initials(rhs)) {
                c(final, initial);
            }
        }
    }

    constexpr auto initials() const { return dsl::initials(lhs); }
    constexpr auto finals() const { return dsl::finals(rhs); }
};

template <node Lhs, node Rhs> seq(Lhs, Rhs) -> seq<Lhs, Rhs>;
} // namespace flow::dsl

template <flow::dsl::node Lhs, flow::dsl::node Rhs>
[[nodiscard]] constexpr auto operator>>(Lhs const &lhs, Rhs const &rhs) {
    return flow::dsl::seq{lhs, rhs};
}
