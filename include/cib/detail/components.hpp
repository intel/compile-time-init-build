#include "compiler.hpp"
#include "config_item.hpp"
#include "meta.hpp"
#include "../tuple.hpp"
#include "type_list.hpp"

#include <type_traits>


#ifndef COMPILE_TIME_INIT_BUILD_COMPONENTS_HPP
#define COMPILE_TIME_INIT_BUILD_COMPONENTS_HPP


namespace cib::detail {
    template<typename... Components>
    struct components : public detail::config_item {
        template<typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto extends_tuple(Args const & ... args) const {
            return tuple_cat(Components::config.extends_tuple(args...)...);
        }
    };
}


#endif //COMPILE_TIME_INIT_BUILD_COMPONENTS_HPP
