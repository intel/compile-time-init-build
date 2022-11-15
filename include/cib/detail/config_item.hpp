#pragma once

#include <cib/tuple.hpp>

namespace cib::detail {
struct config_item {
    template <typename... Args>
    [[nodiscard]] constexpr auto extends_tuple(Args const &...) const {
        return cib::tuple<>{};
    }
};
} // namespace cib::detail
