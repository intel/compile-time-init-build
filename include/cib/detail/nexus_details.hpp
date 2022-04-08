#include "compiler.hpp"
#include "meta.hpp"

#include <type_traits>


#ifndef COMPILE_TIME_INIT_BUILD_NEXUS_DETAILS_HPP
#define COMPILE_TIME_INIT_BUILD_NEXUS_DETAILS_HPP


namespace cib {
    template<typename Config>
    struct initialized_builders {
        CIB_CONSTEXPR static auto value = Config::config.init(Config::config.exports_tuple());
        using type = decltype(value);
    };

    template<typename Config>
    CIB_CONSTEXPR static auto & initialized_builders_v = initialized_builders<Config>::value;

    template<typename Config, typename Tag>
    struct initialized {
        CIB_CONSTEXPR static auto value =
            detail::fold_right(initialized_builders_v<Config>, 0, [](auto b, [[maybe_unused]] auto retval){
                if constexpr (std::is_same_v<decltype(b.first), Tag>) {
                    return b.second;
                } else {
                    return retval;
                }
            });
    };
}


#endif //COMPILE_TIME_INIT_BUILD_NEXUS_DETAILS_HPP
