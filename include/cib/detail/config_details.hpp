#include "compiler.hpp"
#include "config_item.hpp"
#include "meta.hpp"
#include "../tuple.hpp"
#include "type_list.hpp"


#ifndef COMPILE_TIME_INIT_BUILD_DETAIL_CONFIG_HPP
#define COMPILE_TIME_INIT_BUILD_DETAIL_CONFIG_HPP


namespace cib::detail {
    template<auto Value>
    CIB_CONSTEXPR static auto as_constant_v = std::integral_constant<std::remove_cv_t<std::remove_reference_t<decltype(Value)>>, Value>{};

    template<auto... Args>
    struct args {
        static CIB_CONSTEXPR auto value = make_tuple(self_type_index, as_constant_v<Args>...);
    };

    template<typename ConfigArgs, typename... ConfigTs>
    struct config : public detail::config_item {
        decltype(make_tuple(std::declval<ConfigTs>()...)) configs_tuple;

        CIB_CONSTEVAL explicit config(
            ConfigArgs,
            ConfigTs const & ... configs
        )
            : configs_tuple{make_tuple(configs...)}
        {
            // pass
        }

        template<typename BuildersT, typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto init(
            BuildersT const & builders_tuple,
            Args const & ... args
        ) const {
            return apply([&](auto const & ... config_args){
                return fold_right(configs_tuple, builders_tuple, [&](auto const & c, auto builders){
                    return c.init(builders, args..., config_args...);
                });
            }, ConfigArgs::value);
        }

        template<typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto exports_tuple(
            Args const & ... args
        ) const {
            return apply([&](auto const & ... config_args){
                return apply([&](auto const & ... configs_pack){
                    return type_list_cat(configs_pack.exports_tuple(args..., config_args...)...);
                }, configs_tuple);
            }, ConfigArgs::value);
        }
    };
}

#endif //COMPILE_TIME_INIT_BUILD_DETAIL_CONFIG_HPP
