#include "compiler.hpp"
#include "config_item.hpp"
#include "meta.hpp"

#include <tuple>


#ifndef COMPILE_TIME_INIT_BUILD_DETAIL_CONFIG_HPP
#define COMPILE_TIME_INIT_BUILD_DETAIL_CONFIG_HPP


namespace cib::detail {
    template<typename... ConfigTs>
    struct config : public detail::config_item {
        std::tuple<ConfigTs...> configsTuple;

        [[nodiscard]] CIB_CONSTEVAL config(
            ConfigTs const & ... configs
        )
            : configsTuple{configs...}
        {
            // pass
        }

        template<typename BuildersT, typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto init(
            BuildersT const & buildersTuple,
            Args const & ... args
        ) const {
            return detail::fold_right(configsTuple, buildersTuple, [&](auto const & c, auto builders){
                return c.init(builders, args...);
            });
        }

        template<typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto exports_tuple(
            Args const & ... args
        ) const {
            return std::apply([&](auto const & ... configsPack){
                return std::tuple_cat(configsPack.exports_tuple(args...)...);
            }, configsTuple);
        }
    };
}

#endif //COMPILE_TIME_INIT_BUILD_DETAIL_CONFIG_HPP
