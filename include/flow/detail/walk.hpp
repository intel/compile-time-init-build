#pragma once

#include <container/Vector.hpp>
#include <flow/detail/milestone_impl.hpp>

#include <type_traits>

namespace flow::detail {
using FlowComboVector = Vector<milestone_base, 8>;

template <typename T> [[nodiscard]] constexpr auto get_heads(T const &ref) {
    if constexpr (std::is_same_v<milestone_base, T>) {
        return FlowComboVector({ref});
    } else {
        return ref.get_heads();
    }
}

template <typename T> [[nodiscard]] constexpr auto get_tails(T const &ref) {
    if constexpr (std::is_same_v<milestone_base, T>) {
        return FlowComboVector({ref});
    } else {
        return ref.get_tails();
    }
}

template <typename Callable>
constexpr void node_walk(Callable c, milestone_base const &node) {
    c(node, {});
}

template <typename Callable, typename T>
constexpr void node_walk(Callable c, T const &node) {
    node.walk(c);
}
} // namespace flow::detail
