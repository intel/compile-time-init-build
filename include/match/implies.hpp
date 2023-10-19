#pragma once

#include <match/concepts.hpp>

#include <utility>

namespace match {
constexpr inline class implies_t {
    template <matcher X, matcher Y>
    [[nodiscard]] friend constexpr auto tag_invoke(implies_t, X const &,
                                                   Y const &) -> bool {
        return false;
    }

    // NOTE: This overload takes two arguments and uses a requires clause. It
    // does not just take one template argument in order to avoid ambiguity.
    template <matcher M, matcher N>
        requires(std::is_same_v<M, N>)
    [[nodiscard]] friend constexpr auto tag_invoke(implies_t, M const &,
                                                   N const &) -> bool {
        return true;
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
    // noexcept(noexcept(tag_invoke(std::declval<implies_t>(),
    //                              std::forward<Ts>(ts)...)))
    /*            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) */ {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} implies{};
} // namespace match
