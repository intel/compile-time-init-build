#pragma once

#include <lookup/input.hpp>
#include <lookup/strategy_failed.hpp>

#include <stdx/compiler.hpp>
#include <stdx/type_traits.hpp>

#include <algorithm>
#include <array>
#include <iterator>

namespace lookup {
struct fail_strategy_t {
    [[nodiscard]] CONSTEVAL static auto make(compile_time auto)
        -> strategy_failed_t {
        return {};
    }
};

template <typename... Ts> struct strategies {
    [[nodiscard]] CONSTEVAL static auto make(compile_time auto input) {
        constexpr auto idx = [&] {
            constexpr auto results =
                std::array{not strategy_failed(Ts::make(input))...};
            return std::distance(
                std::cbegin(results),
                std::find(std::cbegin(results), std::cend(results), true));
        }();
        return stdx::nth_t<idx, Ts..., fail_strategy_t>::make(input);
    }
};
} // namespace lookup
