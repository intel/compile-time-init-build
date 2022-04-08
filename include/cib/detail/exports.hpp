#include "compiler.hpp"
#include "config_item.hpp"

#include <tuple>
#include <utility>


#ifndef COMPILE_TIME_INIT_BUILD_EXPORTS_HPP
#define COMPILE_TIME_INIT_BUILD_EXPORTS_HPP


namespace cib::detail {
    template<typename... Services>
    struct exports : public detail::config_item {
        template<typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto exports_tuple(Args const & ...) const {
            return std::tuple<std::pair<Services, traits::builder_t<Services>>...>{};
        }
    };
}


#endif //COMPILE_TIME_INIT_BUILD_EXPORTS_HPP
