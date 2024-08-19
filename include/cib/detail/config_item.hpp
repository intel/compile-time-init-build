#pragma once

#include <stdx/tuple.hpp>

namespace cib::detail {
struct config_item {
    template <typename... Args>
    [[nodiscard]] constexpr auto
    extends_tuple(Args const &...) const -> stdx::tuple<> {
        return {};
    }

    template <typename... InitArgs>
    [[nodiscard]] constexpr auto
    exports_tuple(InitArgs const &...) const -> stdx::tuple<> {
        return {};
    }
};
} // namespace cib::detail
