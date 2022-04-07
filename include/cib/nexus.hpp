#include "detail/compiler.hpp"
#include "detail/meta.hpp"
#include "built.hpp"

#include <type_traits>


#ifndef COMPILE_TIME_INIT_BUILD_NEXUS_HPP
#define COMPILE_TIME_INIT_BUILD_NEXUS_HPP


namespace cib {
    template<typename ConfigT>
    struct initialized_builders {
        CIB_CONSTEXPR static auto value = ConfigT::config.init(ConfigT::config.exports_tuple());
        using type = decltype(value);
    };

    template<typename ConfigT>
    using initialized_builders_t = typename initialized_builders<ConfigT>::type;

    template<typename ConfigT>
    CIB_CONSTEXPR static auto & initialized_builders_v = initialized_builders<ConfigT>::value;

    template<typename ConfigT, typename Tag>
    struct initialized {
        CIB_CONSTEXPR static auto value = detail::fold_right(initialized_builders_v<ConfigT>, 0, [](auto b, auto retval){
            if constexpr (std::is_same_v<decltype(b.first), Tag>) {
                return b.second;
            } else {
                return retval;
            }
        });
    };

    /**
     * Type trait for building a Builder and storing its Implementation.
     *
     * @tparam Builders
     *      A type with a 'static constexpr value' field that contains a std::tuple of all the initialized builders.
     *
     * @tparam Tag
     *      The typename of a the Builder to be built into an implementation.
     */
    /**
     * Build the builder. Passing in a type with a 'static constexpr value' member field is a pattern that works
     * for all builder/implementation combinations. This 'value' field is where the built Builder is stored.
     */
    template<typename ConfigT>
    struct nexus {
        template<typename T>
        using builder_t = decltype(initialized<ConfigT, T>::value.template build<initialized<ConfigT, T>>());

        template<typename T>
        CIB_CONSTINIT static inline builder_t<T> builder = initialized<ConfigT, T>::value.template build<initialized<ConfigT, T>>();

        static void init() {
            detail::for_each(initialized_builders_v<ConfigT>, [](auto b){
                // Tag/CleanTag is the type name of the builder_meta in the tuple
                using Tag = decltype(b.first);
                using CleanTag = std::remove_cv_t<std::remove_reference_t<Tag>>;

                // the built implementation is stored in Build<>::value
                auto & builtValue = builder<CleanTag>;
                using BuiltType = std::remove_reference_t<decltype(builtValue)>;

                // if the built type is a pointer, then it is a function pointer and we should return its value
                // directly to the 'built<>' abstract interface variable.
                if constexpr(std::is_pointer_v<BuiltType>) {
                    built<CleanTag> = builtValue;

                // if the built type is not a pointer, then it is a class and the 'built<>' variable is a pointer to
                // the base class. we will need to get a pointer to the builtValue in order to initialize 'built<>'.
                } else {
                    built<CleanTag> = &builtValue;
                }
            });
        }
    };
}


#endif //COMPILE_TIME_INIT_BUILD_NEXUS_HPP
