#pragma once

#include <lookup/strategy_failed.hpp>

namespace lookup {
    template<typename... Ts>
    struct strategies;

    template<>
    struct strategies<> {
        template<typename InputValues>
        [[nodiscard]] consteval static auto make() -> strategy_failed_t {
            return {};
        }

    };

    template<typename T, typename... Ts>
    struct strategies<T, Ts...> {
        template<typename InputValues>
        [[nodiscard]] consteval static auto make() {
            constexpr auto candidate = T::template make<InputValues>();

            if constexpr (strategy_failed(candidate)) {
                return strategies<Ts...>::template make<InputValues>();
            } else {
                return candidate;
            }
        }
    };
}
