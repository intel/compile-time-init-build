#include "compiler.hpp"
#include "type_list.hpp"
#include "../tuple.hpp"


#ifndef COMPILE_TIME_INIT_BUILD_CONFIG_DETAIL_HPP
#define COMPILE_TIME_INIT_BUILD_CONFIG_DETAIL_HPP


namespace cib::detail {
    struct config_item {
        template<typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto exports_tuple(Args const & ...) const {
            return type_list<>{};
        }

        template<typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto extends_tuple(Args const & ...) const {
            return cib::tuple<>{};
        }
    };
}


#endif //COMPILE_TIME_INIT_BUILD_CONFIG_DETAIL_HPP
