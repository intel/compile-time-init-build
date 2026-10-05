#pragma once

#include <match/concepts.hpp>

#include <utility>

namespace match {
template <matcher> struct not_t;

constexpr inline struct negate_t {
    template <matcher M>
    [[nodiscard]] constexpr auto operator()(M &&m) const
        -> not_t<std::remove_cvref_t<M>> {
        return {std::forward<M>(m)};
    }

    template <matcher M>
        requires true
    [[nodiscard]] constexpr auto operator()(M &&m) const
        noexcept(noexcept(std::forward<M>(m).negate()))
            -> decltype(std::forward<M>(m).negate()) {
        return std::forward<M>(m).negate();
    }
} negate{};
} // namespace match
