#include "compiler.hpp"
#include "meta.hpp"
#include "type_list.hpp"
#include "tuple.hpp"

#include <type_traits>


#ifndef COMPILE_TIME_INIT_BUILD_NEXUS_DETAILS_HPP
#define COMPILE_TIME_INIT_BUILD_NEXUS_DETAILS_HPP


namespace cib {
    template<typename ServiceBuilderList>
    struct to_tuple;

    template<typename... ServiceBuilders>
    struct to_tuple<detail::type_list<ServiceBuilders...>> {
        using type = detail::tuple<ServiceBuilders...>;
        constexpr static inline auto value = type{ServiceBuilders{}...};
    };

    template<typename ServiceBuilderList>
    using to_tuple_t = typename to_tuple<ServiceBuilderList>::type;

    template<typename ServiceBuilderList>
    constexpr static auto to_tuple_v = to_tuple<ServiceBuilderList>::value;

    template<typename Config>
    struct initialized_builders {
        CIB_CONSTEXPR static auto value = Config::config.init(to_tuple_v<decltype(Config::config.exports_tuple())>);
        using type = decltype(value);
    };

    template<typename Config>
    CIB_CONSTEXPR static auto & initialized_builders_v = initialized_builders<Config>::value;

    template<typename Config, typename Tag>
    struct initialized {
        CIB_CONSTEXPR static auto value =
            detail::fold_right(initialized_builders_v<Config>, 0, [](auto b, [[maybe_unused]] auto retval){
                if constexpr (std::is_same_v<typename decltype(b)::Service, Tag>) {
                    return b.builder;
                } else {
                    return retval;
                }
            });
    };
}


#endif //COMPILE_TIME_INIT_BUILD_NEXUS_DETAILS_HPP
