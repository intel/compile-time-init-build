#pragma once

#include <lookup/input.hpp>
#include <lookup/strategy_failed.hpp>

#include <stdx/compiler.hpp>

namespace lookup {
template <typename...> struct strategies;

template <> struct strategies<> {
    [[nodiscard]] CONSTEVAL static auto make(compile_time auto)
        -> strategy_failed_t {
        return {};
    }
};

template <typename T, typename... Ts> struct strategies<T, Ts...> {
    [[nodiscard]] CONSTEVAL static auto make(compile_time auto input) {
        constexpr auto candidate = T::make(input);

        if constexpr (strategy_failed(candidate)) {
            return strategies<Ts...>::make(input);
        } else {
            return candidate;
        }
    }
};
} // namespace lookup
