#pragma once

#include <stdx/tuple.hpp>

namespace cib::detail {
struct config_item {
    [[nodiscard]] constexpr auto extends_tuple() const -> stdx::tuple<> {
        return {};
    }

    [[nodiscard]] constexpr auto exports_tuple() const -> stdx::tuple<> {
        return {};
    }
};
} // namespace cib::detail
