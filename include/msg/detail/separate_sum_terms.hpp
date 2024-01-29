#pragma once

#include <match/and.hpp>
#include <match/concepts.hpp>
#include <match/or.hpp>
#include <match/sum_of_products.hpp>

#include <stdx/concepts.hpp>
#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>

#include <type_traits>
#include <utility>

namespace msg {
namespace detail {
template <match::matcher M, typename C>
    requires(
        not stdx::is_specialization_of_v<std::remove_cvref_t<M>, match::or_t>)
constexpr auto separate_sum_terms(M &&m, C &&c) {
    using matcher_t = std::remove_cvref_t<M>;
    using callback_t = std::remove_cvref_t<C>;
    return stdx::make_tuple(
        typename callback_t::template rebind_matcher<matcher_t>{
            std::forward<M>(m), std::forward<C>(c).callable});
}

template <match::matcher M>
    requires stdx::is_specialization_of_v<std::remove_cvref_t<M>, match::or_t>
constexpr auto separate_sum_terms(M &&m, auto const &c) {
    return stdx::tuple_cat(separate_sum_terms(std::forward<M>(m).lhs, c),
                           separate_sum_terms(std::forward<M>(m).rhs, c));
}
} // namespace detail

template <typename C, match::matcher... Ms>
constexpr auto separate_sum_terms(C &&c, Ms &&...ms) {
    auto m = match::sum_of_products(
        match::all(std::forward<C>(c).matcher, std::forward<Ms>(ms)...));
    return detail::separate_sum_terms(std::move(m), std::forward<C>(c));
}
} // namespace msg
