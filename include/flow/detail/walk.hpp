#pragma once

#include <stdx/concepts.hpp>
#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>

#include <utility>

namespace flow::dsl {
template <typename T>
concept node = requires { typename stdx::remove_cvref_t<T>::is_node; };

template <typename Source, typename Dest> struct edge {
    using source_t = Source;
    using dest_t = Dest;
};

constexpr inline class get_initials_t {
    template <node N> friend constexpr auto tag_invoke(get_initials_t, N &&n) {
        return stdx::make_tuple(std::forward<N>(n));
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
    template <node N> friend constexpr auto tag_invoke(get_finals_t, N &&n) {
        return stdx::make_tuple(std::forward<N>(n));
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

constexpr inline class get_nodes_t {
    template <node N> friend constexpr auto tag_invoke(get_nodes_t, N &&n) {
        return stdx::make_tuple(std::forward<N>(n));
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<get_nodes_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} get_nodes{};

constexpr inline class get_edges_t {
    friend constexpr auto tag_invoke(get_edges_t, node auto const &) {
        return stdx::tuple{};
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<get_edges_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} get_edges{};
} // namespace flow::dsl
