#include <type_traits>


#ifndef COMPILE_TIME_INIT_BUILD_TYPE_PACK_ELEMENT_HPP
#define COMPILE_TIME_INIT_BUILD_TYPE_PACK_ELEMENT_HPP


namespace cib::detail {
    #if defined(__clang__)
        template<auto Index, typename... Tn>
        using type_pack_element = __type_pack_element<Index, Tn...>;
    #else
        template<int Index, typename T, typename... Tn>
        struct get_type_pack_element {
            using type = typename get_type_pack_element<Index - 1, Tn...>::type;
        };

        template<typename T, typename... Tn>
        struct get_type_pack_element<0, T, Tn...> {
            using type = T;
        };

        template<int Index, typename... Tn>
        using type_pack_element = typename get_type_pack_element<Index, Tn...>::type;
    #endif
}


#endif //COMPILE_TIME_INIT_BUILD_TYPE_PACK_ELEMENT_HPP
