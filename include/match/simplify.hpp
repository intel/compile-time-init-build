#pragma once

#include <match/concepts.hpp>

#include <utility>

namespace match {
constexpr inline class simplify_t {
    template <matcher M>
    [[nodiscard]] friend constexpr auto tag_invoke(simplify_t, M const &m)
        -> M {
        return m;
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<simplify_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} simplify{};
} // namespace match
