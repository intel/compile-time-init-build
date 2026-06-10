#pragma once

#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>

namespace cib::detail {
struct config_item {
    [[nodiscard]] constexpr static auto extends_tuple() -> stdx::tuple<> {
        return {};
    }

    [[nodiscard]] constexpr static auto get_exports() -> stdx::type_list<> {
        return {};
    }
};
} // namespace cib::detail
