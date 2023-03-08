#pragma once

#include <cib/tuple.hpp>

namespace cib::detail {
struct config_item {
    template <typename... Args>
    [[nodiscard]] constexpr auto extends_tuple(Args const &...) const {
        return cib::make_tuple();
    }

    template <typename... InitArgs>
    [[nodiscard]] constexpr auto exports_tuple(InitArgs const &...) const {
        return cib::make_tuple();
    }
};
} // namespace cib::detail
