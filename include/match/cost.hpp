#pragma once

#include <match/concepts.hpp>

#include <cstddef>
#include <utility>

namespace match {
constexpr inline class cost_t {
    [[nodiscard]] friend constexpr auto tag_invoke(cost_t, auto const &)
        -> std::size_t {
        return 1u;
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<cost_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} cost{};
} // namespace match
