#pragma once

#include <stdx/concepts.hpp>

#include <array>
#include <utility>

namespace flow::dsl {
template <typename T>
concept node = requires { typename T::is_node; };

constexpr inline class walk_t {
    template <node N, stdx::invocable<N> F>
    friend constexpr auto tag_invoke(walk_t, F &&f, N const &n) {
        return std::forward<F>(f)(n);
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<walk_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} walk{};

constexpr inline class get_initials_t {
    friend constexpr auto tag_invoke(get_initials_t, node auto const &n) {
        return std::array{n};
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<get_initials_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} get_initials{};

constexpr inline class get_finals_t {
    friend constexpr auto tag_invoke(get_finals_t, node auto const &n) {
        return std::array{n};
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<get_finals_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} get_finals{};
} // namespace flow::dsl
