#pragma once

#include <lookup/strategy_failed.hpp>

#include <stdx/compiler.hpp>

namespace lookup {
template <typename...> struct strategies;

template <> struct strategies<> {
    template <typename>
    [[nodiscard]] CONSTEVAL static auto make() -> strategy_failed_t {
        return {};
    }
};

template <typename T, typename... Ts> struct strategies<T, Ts...> {
    template <typename InputValues> [[nodiscard]] CONSTEVAL static auto make() {
        constexpr auto candidate = T::template make<InputValues>();

        if constexpr (strategy_failed(candidate)) {
            return strategies<Ts...>::template make<InputValues>();
        } else {
            return candidate;
        }
    }
};
} // namespace lookup
