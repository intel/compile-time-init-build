#include "builder_meta.hpp"

#include "detail/compiler.hpp"
#include "detail/meta.hpp"
#include "detail/builder_traits.hpp"
#include "detail/config.hpp"
#include "detail/conditional.hpp"
#include "detail/components.hpp"
#include "detail/exports.hpp"
#include "detail/extend.hpp"
#include "detail/config_item.hpp"

#include <tuple>
#include <type_traits>


#ifndef COMPILE_TIME_INIT_BUILD_CONFIG_HPP
#define COMPILE_TIME_INIT_BUILD_CONFIG_HPP


namespace cib {
    template<auto... Args>
    using args = detail::args<Args...>;

    template<typename ArgType>
    CIB_CONSTEXPR static detail::arg<ArgType> arg{};

    template<typename... ConfigTs>
    [[nodiscard]] CIB_CONSTEVAL auto config(ConfigTs const & ... configs) {
        return detail::config{configs...};
    }

    template<typename Args, typename... Services>
    CIB_CONSTEXPR static detail::components<Args, Services...> components{};

    template<typename... Services>
    CIB_CONSTEXPR static detail::exports<Services...> exports{};

    template<
        typename... ServicePaths,
        typename... Args>
    [[nodiscard]] CIB_CONSTEVAL auto extend(Args const & ... args) {
        return detail::extend_t<detail::path<ServicePaths...>, Args...>{args...};
    }

    template<
        typename Condition,
        typename... Configs>
    [[nodiscard]] CIB_CONSTEVAL auto conditional(
        Condition const & condition,
        Configs const & ... configs
    ) {
        return detail::conditional{condition, configs...};
    }
}


#endif //COMPILE_TIME_INIT_BUILD_CONFIG_HPP
