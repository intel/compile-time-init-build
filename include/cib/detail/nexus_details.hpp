#include <cib/detail/compiler.hpp>
#include <cib/detail/meta.hpp>
#include <cib/detail/type_list.hpp>
#include <cib/tuple.hpp>
#include <cib/set.hpp>
#include <cib/detail/exports.hpp>


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
        using type = cib::tuple<index_metafunc_t<extract_service_tag>, ServiceBuilders...>;
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
                    constexpr auto initial_builder = extensions.get(index_<0>).builder;
                    using service = get_service_from_tuple::invoke<decltype(extensions)>;
                    auto built_service = extensions.fold_right(initial_builder, [](auto extension, auto outer_builder){
                        return extension.args_tuple.fold_right(outer_builder, [](auto arg, auto inner_builder) {
                            return inner_builder.add(arg);
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
