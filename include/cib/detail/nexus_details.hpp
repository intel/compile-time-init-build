#include "compiler.hpp"
#include "meta.hpp"
#include "type_list.hpp"
#include "indexed_tuple.hpp"
#include "exports.hpp"


#ifndef COMPILE_TIME_INIT_BUILD_NEXUS_DETAILS_HPP
#define COMPILE_TIME_INIT_BUILD_NEXUS_DETAILS_HPP


namespace cib {
    struct extract_service_tag {
        template<typename T>
        using invoke = typename T::Service;
    };

    template<typename ServiceBuilderList>
    struct to_tuple;

    template<typename... ServiceBuilders>
    struct to_tuple<detail::type_list<ServiceBuilders...>> {
        using type = decltype(detail::make_indexed_tuple(detail::index_metafunc_<extract_service_tag>{}, ServiceBuilders{}...));
        constexpr static inline type value = detail::make_indexed_tuple(detail::index_metafunc_<extract_service_tag>{}, ServiceBuilders{}...);
    };

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
            initialized_builders_v<Config>.get(cib::detail::tag_<Tag>{}).builder;
    };

}


#endif //COMPILE_TIME_INIT_BUILD_NEXUS_DETAILS_HPP
