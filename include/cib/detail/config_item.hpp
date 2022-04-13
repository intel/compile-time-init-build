#include "compiler.hpp"
#include "ordered_set.hpp"
#include "type_list.hpp"

#include <tuple>


#ifndef COMPILE_TIME_INIT_BUILD_CONFIG_DETAIL_HPP
#define COMPILE_TIME_INIT_BUILD_CONFIG_DETAIL_HPP


namespace cib::detail {
    template<typename FirstT, typename... PathSegmentTs>
    struct path {
        using First = FirstT;
    };

    struct config_item {
        template<typename Builders, typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto init(
            Builders const & builders_tuple,
            Args const & ...
        ) const {
            return builders_tuple;
        }

        template<typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto exports_tuple(Args const & ...) const {
            return type_list<>{};
        }
    };
}


#endif //COMPILE_TIME_INIT_BUILD_CONFIG_DETAIL_HPP
