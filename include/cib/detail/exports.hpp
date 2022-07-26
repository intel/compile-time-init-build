#include "compiler.hpp"
#include "config_item.hpp"
#include "type_list.hpp"
#include "builder_traits.hpp"
#include "extend.hpp"

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
        template<typename... InitArgs>
        [[nodiscard]] CIB_CONSTEVAL auto extends_tuple(InitArgs const & ...) const {
            return cib::make_tuple(extend<Services>{}...);
        }
    };
}


#endif //COMPILE_TIME_INIT_BUILD_EXPORTS_HPP

