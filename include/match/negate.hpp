#pragma once

#include <match/concepts.hpp>

#include <utility>

namespace match {
template <matcher> struct not_t;

constexpr inline class negate_t {
    template <matcher M>
    [[nodiscard]] friend constexpr auto tag_invoke(negate_t, M const &m)
        -> not_t<M> {
        return {m};
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<negate_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} negate{};
} // namespace match
