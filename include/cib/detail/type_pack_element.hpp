#include <type_traits>
#include <utility>

#ifndef COMPILE_TIME_INIT_BUILD_TYPE_PACK_ELEMENT_HPP
#define COMPILE_TIME_INIT_BUILD_TYPE_PACK_ELEMENT_HPP

namespace cib::detail {
#if defined(__clang__)
template <auto Index, typename... Tn>
using type_pack_element = __type_pack_element<Index, Tn...>;
#else
template <class, int> struct type_id {};
template <class... Ts> struct inherit : Ts... {};
template <int Index, class T>
auto get_type_pack_element_impl(type_id<T, Index>) -> T;
template <int Index, typename... Ts, auto... Ns>
auto get_type_pack_element_impl(std::index_sequence<Ns...>) -> decltype(
    get_type_pack_element_impl<Index>(inherit<type_id<Ts, Ns>...>{}));
template <int Index, typename... Ts> struct get_type_pack_element {
    using type = decltype(get_type_pack_element_impl<Index, Ts...>(
        std::make_index_sequence<sizeof...(Ts)>{}));
};

template <int Index, typename... Tn>
using type_pack_element = typename get_type_pack_element<Index, Tn...>::type;
#endif
} // namespace cib::detail

#endif // COMPILE_TIME_INIT_BUILD_TYPE_PACK_ELEMENT_HPP
