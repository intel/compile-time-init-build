#pragma once

#include <flow/detail/walk.hpp>

namespace flow::dsl {
template <node Lhs, node Rhs> struct seq {
    Lhs lhs;
    Rhs rhs;

    using is_node = void;

  private:
    template <typename F>
    friend constexpr auto tag_invoke(walk_t, F &&f, seq const &s) -> void {
        walk(f, s.lhs);
        walk(f, s.rhs);

        for (auto final : get_finals(s.lhs)) {
            for (auto initial : get_initials(s.rhs)) {
                f(final, initial);
            }
        }
    }

    friend constexpr auto tag_invoke(get_initials_t, seq const &s) {
        return get_initials(s.lhs);
    }

    friend constexpr auto tag_invoke(get_finals_t, seq const &s) {
        return get_finals(s.rhs);
    }
};

template <node Lhs, node Rhs> seq(Lhs, Rhs) -> seq<Lhs, Rhs>;
} // namespace flow::dsl

template <flow::dsl::node Lhs, flow::dsl::node Rhs>
[[nodiscard]] constexpr auto operator>>(Lhs const &lhs, Rhs const &rhs) {
    return flow::dsl::seq{lhs, rhs};
}
