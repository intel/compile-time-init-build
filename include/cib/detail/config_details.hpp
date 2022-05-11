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
        static CIB_CONSTEXPR auto value = cib::make_tuple(self_type_index, as_constant_v<Args>...);
    };

    template<typename ConfigArgs, typename... ConfigTs>
    struct config : public detail::config_item {
        cib::tuple<ConfigTs...> configs_tuple;

        CIB_CONSTEVAL explicit config(
            ConfigArgs,
            ConfigTs const & ... configs
        )
            : configs_tuple{cib::make_tuple(configs...)}
        {
            // pass
        }

        template<typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto extends_tuple(Args const & ... args) const {
            return apply([&](auto const & ... config_args){
                return apply([&](auto const & ... configs_pack){
                    return tuple_cat(configs_pack.extends_tuple(args..., config_args...)...);
                }, configs_tuple);
            }, ConfigArgs::value);
        }
    };
}

#endif //COMPILE_TIME_INIT_BUILD_DETAIL_CONFIG_HPP
