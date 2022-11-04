#include <cib/detail/compiler.hpp>
#include <cib/detail/type_list.hpp>
#include <cib/tuple.hpp>

#ifndef COMPILE_TIME_INIT_BUILD_CONFIG_DETAIL_HPP
#define COMPILE_TIME_INIT_BUILD_CONFIG_DETAIL_HPP

namespace cib::detail {
struct config_item {
    template <typename... Args>
    [[nodiscard]] CIB_CONSTEXPR auto extends_tuple(Args const &...) const {
        return cib::tuple<>{};
    }
};
} // namespace cib::detail

#endif // COMPILE_TIME_INIT_BUILD_CONFIG_DETAIL_HPP
