#include "compiler.hpp"
#include "meta.hpp"
#include "type_list.hpp"
#include "../tuple.hpp"
#include "../set.hpp"
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
        using type = tuple<index_metafunc_t<extract_service_tag>, ServiceBuilders...>;
        constexpr static inline type value{};
    };

    template<typename ServiceBuilderList>
    constexpr static auto to_tuple_v = to_tuple<ServiceBuilderList>::value;

    struct get_service {
        template<typename T>
        using invoke = typename std::remove_cv_t<std::remove_reference_t<T>>::service_type;
    };

    struct get_service_from_tuple {
        template<typename T>
        using invoke =
            typename std::remove_cv_t<
                std::remove_reference_t<
                    decltype(
                        std::declval<T>().get(index_<0>)
                    )
                >
            >::service_type;
    };

    template<typename Config>
    struct initialized_builders {
        CIB_CONSTEXPR static auto value =
            transform(
                index_metafunc_<extract_service_tag>,
                demux(get_service{}, Config::config.extends_tuple()),
                [](auto extensions){
                    using detail::int_;
                    constexpr auto initial_builder = extensions.get(index_<0>).builder;
                    using service = get_service_from_tuple::invoke<decltype(extensions)>;
                    constexpr auto built_service = detail::fold_right(extensions, initial_builder, [](auto extension, auto b){
                        return extension.args_tuple.apply([&](auto... args){
                            return b.add(args...);
                        });
                    });

                    return detail::service_entry<service, decltype(built_service)>{built_service};
                }
            );

        using type = decltype(value);
    };

    template<typename Config>
    CIB_CONSTEXPR static auto & initialized_builders_v = initialized_builders<Config>::value;

    template<typename Config, typename Tag>
    struct initialized {
        CIB_CONSTEXPR static auto value =
            initialized_builders_v<Config>.get(cib::tag_<Tag>).builder;
    };

}


#endif //COMPILE_TIME_INIT_BUILD_NEXUS_DETAILS_HPP
