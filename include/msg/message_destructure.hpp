#pragma once

#include <msg/message.hpp>

#include <stdx/type_traits.hpp>

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

template <msg::messagelike M>
struct std::tuple_size<M> : M::definition_t::num_fields_t {};

template <std::size_t I, msg::messagelike M>
struct std::tuple_element<I, M>
    : std::type_identity<
          typename M::definition_t::template nth_field_t<I>::value_type> {};

namespace msg::detail {
template <std::size_t I, msg::messagelike M>
constexpr auto get(M &&m) -> decltype(auto) {
    return std::forward<M>(m).get(typename std::remove_cvref_t<
                                  M>::definition_t::template nth_field_t<I>{});
}
} // namespace msg::detail
