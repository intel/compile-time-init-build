#pragma once

#include <match/concepts.hpp>

#include <stdx/concepts.hpp>

#include <utility>

namespace match {
constexpr inline struct implies_t {
    template <matcher X, matcher Y>
    [[nodiscard]] constexpr auto operator()(X &&x, Y &&y) const noexcept
        -> bool {
        if constexpr (stdx::same_as_unqualified<X, Y>) {
            return true;
        } else if constexpr (requires {
                                 std::forward<X>(x).implies(std::forward<Y>(y));
                             }) {
            return std::forward<X>(x).implies(std::forward<Y>(y));
        } else if constexpr (requires {
                                 std::forward<Y>(y).implied_by(
                                     std::forward<X>(x));
                             }) {
            return std::forward<Y>(y).implied_by(std::forward<X>(x));
        }
        return false;
    }
} implies{};
} // namespace match
