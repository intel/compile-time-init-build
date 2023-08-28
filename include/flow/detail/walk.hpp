#pragma once

#include <array>

namespace flow::dsl {
template <typename T>
concept node = requires { typename T::is_node; };

[[nodiscard]] constexpr auto initials(node auto const &n) {
    if constexpr (requires { n.initials(); }) {
        return n.initials();
    } else {
        return std::array{n};
    }
}

[[nodiscard]] constexpr auto finals(node auto const &n) {
    if constexpr (requires { n.finals(); }) {
        return n.finals();
    } else {
        return std::array{n};
    }
}

constexpr auto walk(auto c, node auto const &n) -> void {
    if constexpr (requires { n.walk(c); }) {
        n.walk(c);
    } else {
        c(n, {});
    }
}
} // namespace flow::dsl
