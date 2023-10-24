#pragma once

#include <match/and.hpp>
#include <match/concepts.hpp>
#include <match/sum_of_products.hpp>
#include <msg/field_matchers.hpp>

#include <stdx/concepts.hpp>
#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>

#include <type_traits>
#include <utility>

namespace msg {
struct null_matcher_validator {
    template <match::matcher M>
    CONSTEVAL static auto validate() noexcept -> bool {
        return true;
    }
};

struct never_matcher_validator {
    template <match::matcher M>
    CONSTEVAL static auto validate() noexcept -> bool {
        return not std::is_same_v<M, match::never_t>;
    }
};

template <typename...> inline auto matcher_validator = null_matcher_validator{};

template <match::matcher M, typename... DummyArgs>
CONSTEVAL auto validate_matcher() -> bool {
    return matcher_validator<DummyArgs...>.template validate<M>();
}

template <typename Name, match::matcher M, stdx::callable F>
struct indexed_callback_t {
    using name_t = Name;
    using matcher_t = M;
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

namespace detail {
template <typename Name, match::matcher M, typename F>
constexpr auto separate_sum_terms(M &&m, F &&f) {
    using matcher_t = std::remove_cvref_t<M>;
    using callable_t = std::remove_cvref_t<F>;
    if constexpr (stdx::is_specialization_of_v<matcher_t, match::or_t>) {
        auto lcb =
            indexed_callback_t<Name, typename matcher_t::lhs_t, callable_t>{
                std::forward<M>(m).lhs, f};
        auto rcb =
            indexed_callback_t<Name, typename matcher_t::rhs_t, callable_t>{
                std::forward<M>(m).rhs, f};
        return stdx::tuple_cat(separate_sum_terms(std::move(lcb)),
                               separate_sum_terms(std::move(rcb)));
    } else {
        return stdx::make_tuple(indexed_callback_t<Name, matcher_t, callable_t>{
            std::forward<M>(m), std::forward<F>(f)});
    }
}
} // namespace detail

template <typename C, match::matcher... Ms>
constexpr auto separate_sum_terms(C &&c, Ms &&...ms) {
    using callback_t = std::remove_cvref_t<C>;
    auto m = match::sum_of_products(
        match::all(std::forward<C>(c).matcher, std::forward<Ms>(ms)...));
    return detail::separate_sum_terms<typename callback_t::name_t>(
        std::move(m), std::forward<C>(c).callable);
}
} // namespace msg
