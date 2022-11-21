#pragma once

#include <container/Vector.hpp>

#include <type_traits>

namespace flow::detail {
template <typename NodeType> using FlowComboVector = Vector<NodeType, 8>;

template <typename NodeType, typename T>
[[nodiscard]] constexpr auto get_heads(T const &ref) {
    if constexpr (std::is_same_v<NodeType, T>) {
        return FlowComboVector<NodeType>({ref});
    } else {
        return ref.get_heads();
    }
}

template <typename NodeType, typename T>
[[nodiscard]] constexpr auto get_tails(T const &ref) {
    if constexpr (std::is_same_v<NodeType, T>) {
        return FlowComboVector<NodeType>({ref});
    } else {
        return ref.get_tails();
    }
}

template <typename NodeType, typename Callable, typename T>
constexpr void node_walk(Callable c, T const &node) {
    if constexpr (std::is_same_v<NodeType, T>) {
        c(node, {});
    } else {
        node.walk(c);
    }
}
} // namespace flow::detail
