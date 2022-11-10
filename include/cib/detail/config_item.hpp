#pragma once

#include <cib/detail/compiler.hpp>
#include <cib/tuple.hpp>

namespace cib::detail {
struct config_item {
    template <typename... Args>
    [[nodiscard]] CIB_CONSTEXPR auto extends_tuple(Args const &...) const {
        return cib::tuple<>{};
    }
};
} // namespace cib::detail
