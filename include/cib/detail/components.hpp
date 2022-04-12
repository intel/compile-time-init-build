#include "compiler.hpp"
#include "config_item.hpp"
#include "meta.hpp"
#include "ordered_set.hpp"
#include "type_list.hpp"

#include <type_traits>


#ifndef COMPILE_TIME_INIT_BUILD_COMPONENTS_HPP
#define COMPILE_TIME_INIT_BUILD_COMPONENTS_HPP


namespace cib::detail {
    template<typename ComponentArgs, typename... Components>
    struct components : public detail::config_item {
        template<typename Builders, typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto init(
            Builders const & builders_tuple,
            Args const & ... args
        ) const {
            return apply([&](auto const & ... component_args){
                return detail::fold_right(ordered_set{Components{}...}, builders_tuple, [&](auto const & c, auto const & builders){
                    return c.config.init(builders, args..., component_args...);
                });
            }, ComponentArgs::value);
        }

        template<typename... Args>
        [[nodiscard]] CIB_CONSTEVAL auto exports_tuple(
            Args const & ... args
        ) const {
            return apply([&](auto const & ... component_args){
                return type_list_cat(Components::config.exports_tuple(args..., component_args...)...);
            }, ComponentArgs::value);
        }
    };

    template<auto Value>
    CIB_CONSTEXPR static auto as_constant_v = std::integral_constant<std::remove_cv_t<std::remove_reference_t<decltype(Value)>>, Value>{};

    template<auto... Args>
    struct args {
        static CIB_CONSTEXPR auto value = ordered_set{as_constant_v<Args>...};
    };
}


#endif //COMPILE_TIME_INIT_BUILD_COMPONENTS_HPP
