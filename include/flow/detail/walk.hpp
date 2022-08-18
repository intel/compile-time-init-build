#pragma once


#include <flow/detail/milestone_impl.hpp>

#include <type_traits>


namespace flow::detail {
    template<typename T>
    [[nodiscard]] constexpr auto get_heads(T const & ref) {
        if constexpr (std::is_same_v<milestone_base, T>) {
            return FlowComboVector({&ref});
        } else {
            return ref.get_heads();
        }
    }

    template<typename T>
    [[nodiscard]] constexpr auto get_heads(T const * ptr) {
        if constexpr (std::is_same_v<milestone_base, T>) {
            return FlowComboVector({ptr});
        } else {
            return ptr->get_heads();
        }
    }

    template<typename T>
    [[nodiscard]] constexpr auto get_tails(T const & ref) {
        if constexpr (std::is_same_v<milestone_base, T>) {
            return FlowComboVector({&ref});
        } else {
            return ref.get_tails();
        }
    }

    template<typename T>
    [[nodiscard]] constexpr auto get_tails(T const * ptr) {
        if constexpr (std::is_same_v<milestone_base, T>) {
            return FlowComboVector({ptr});
        } else {
            return ptr->get_tails();
        }
    }

    template<typename Callable>
    constexpr void node_walk(Callable c, milestone_base const * node) {
        c(node, nullptr);
    }

    template<typename Callable, typename T>
    constexpr void node_walk(Callable c, T node) {
        node.walk(c);
    }
}





