#pragma once

#include <flow/subgraph_identity.hpp>

#include <stdx/concepts.hpp>
#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>

#include <utility>

namespace flow::dsl {
template <typename T>
concept subgraph = requires { typename stdx::remove_cvref_t<T>::is_subgraph; };

template <typename Source, typename Dest, typename Cond> struct edge {
    using source_t = Source;
    using dest_t = Dest;
    using cond_t = Cond;
};

constexpr inline class get_initials_t {
    template <subgraph N>
    friend constexpr auto tag_invoke(get_initials_t, N &&n) {
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
    template <subgraph N>
    friend constexpr auto tag_invoke(get_finals_t, N &&n) {
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
    template <subgraph N> friend constexpr auto tag_invoke(get_nodes_t, N &&n) {
        if constexpr (std::remove_cvref_t<N>::identity ==
                      subgraph_identity::REFERENCE) {
            return stdx::tuple{};
        } else {
            return stdx::make_tuple(std::forward<N>(n));
        }
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

constexpr inline class get_all_mentioned_nodes_t {
    template <subgraph N>
    friend constexpr auto tag_invoke(get_all_mentioned_nodes_t, N &&n) {
        return stdx::make_tuple(std::forward<N>(n));
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<get_all_mentioned_nodes_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} get_all_mentioned_nodes{};

constexpr inline class get_edges_t {
    friend constexpr auto tag_invoke(get_edges_t, subgraph auto const &) {
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
