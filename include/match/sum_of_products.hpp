#pragma once

#include <match/concepts.hpp>

#include <utility>

namespace match {
constexpr inline class sum_of_products_t {
    template <matcher M>
    [[nodiscard]] friend constexpr auto tag_invoke(sum_of_products_t,
                                                   M const &m) -> M {
        return m;
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<sum_of_products_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} sum_of_products{};
} // namespace match
