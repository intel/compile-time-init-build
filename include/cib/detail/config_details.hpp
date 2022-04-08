#include "compiler.hpp"
#include "config_item.hpp"
#include "meta.hpp"

#include <tuple>


#ifndef COMPILE_TIME_INIT_BUILD_DETAIL_CONFIG_HPP
#define COMPILE_TIME_INIT_BUILD_DETAIL_CONFIG_HPP


namespace cib::detail {
    template<typename... ConfigTs>
    struct [[nodiscard]] config : public detail::config_item {
        std::tuple<ConfigTs...> configs_tuple;

        CIB_CONSTEVAL explicit config(
            ConfigTs const & ... configs
        )
            : configs_tuple{configs...}
        {
            // pass
        }

        template<typename BuildersT, typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto init(
            BuildersT const & builders_tuple,
            Args const & ... args
        ) const {
            return detail::fold_right(configs_tuple, builders_tuple, [&](auto const & c, auto builders){
                return c.init(builders, args...);
            });
        }

        template<typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto exports_tuple(
            Args const & ... args
        ) const {
            return std::apply([&](auto const & ... configs_pack){
                return std::tuple_cat(configs_pack.exports_tuple(args...)...);
            }, configs_tuple);
        }
    };
}

#endif //COMPILE_TIME_INIT_BUILD_DETAIL_CONFIG_HPP
