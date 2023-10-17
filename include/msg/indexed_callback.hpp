#pragma once

#include <match/concepts.hpp>
#include <match/sum_of_products.hpp>
#include <msg/field_matchers.hpp>

#include <stdx/concepts.hpp>

#include <type_traits>
#include <utility>

namespace msg {
template <typename Name, match::matcher M, stdx::callable F>
struct indexed_callback_t {
    using name_t = Name;
    using callable_t = F;

    constexpr static Name name{};
    M matcher;
    F callable;
};

constexpr auto indexed_callback =
    []<typename Name, match::matcher M, stdx::callable F>(Name, M &&m, F &&f) {
        auto sop = match::sum_of_products(std::forward<M>(m));
        return indexed_callback_t<Name, decltype(sop), std::remove_cvref_t<F>>{
            std::move(sop), std::forward<F>(f)};
    };

template <typename... Fields>
constexpr auto remove_match_terms = []<typename C>(C &&c) {
    using callback_t = std::remove_cvref_t<C>;
    match::matcher auto new_matcher = remove_terms(
        std::forward<C>(c).matcher, std::type_identity<Fields>{}...);
    return indexed_callback_t<typename callback_t::name_t,
                              decltype(new_matcher),
                              typename callback_t::callable_t>{
        std::move(new_matcher), std::forward<C>(c).callable};
};
} // namespace msg
