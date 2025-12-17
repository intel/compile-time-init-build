#pragma once

#include <stdx/tuple.hpp>

namespace cib::detail {
struct config_item {
    [[nodiscard]] constexpr static auto extends_tuple() -> stdx::tuple<> {
        return {};
    }

    [[nodiscard]] constexpr static auto exports_tuple() -> stdx::tuple<> {
        return {};
    }
};
} // namespace cib::detail
