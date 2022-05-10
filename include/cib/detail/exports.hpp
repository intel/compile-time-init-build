#include "compiler.hpp"
#include "config_item.hpp"
#include "type_list.hpp"
#include "builder_traits.hpp"

#include <utility>


#ifndef COMPILE_TIME_INIT_BUILD_EXPORTS_HPP
#define COMPILE_TIME_INIT_BUILD_EXPORTS_HPP


namespace cib::detail {
    template<
        typename ServiceT,
        typename BuilderT>
    struct service_entry {
        using Service = ServiceT;
        BuilderT builder;
    };



    template<typename... Services>
    struct exports : public detail::config_item {
        template<typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto exports_tuple(Args const & ...) const {
            return type_list<service_entry<Services, traits::builder_t<Services>>...>{};
        }
    };
}


#endif //COMPILE_TIME_INIT_BUILD_EXPORTS_HPP

