#pragma once

#include <match/and.hpp>
#include <match/concepts.hpp>
#include <match/sum_of_products.hpp>
#include <msg/callback.hpp>
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

template <stdx::ct_string Name, typename... ExtraArgs>
constexpr auto indexed_callback =
    []<match::matcher M, stdx::callable F>(M &&m, F &&f) {
        return callback<Name, ExtraArgs...>(
            match::sum_of_products(std::forward<M>(m)), std::forward<F>(f));
    };

template <typename... Fields>
constexpr auto remove_match_terms = []<typename C>(C &&c) {
    using callback_t = std::remove_cvref_t<C>;
    match::matcher auto new_matcher = remove_terms(
        std::forward<C>(c).matcher, std::type_identity<Fields>{}...);
    return detail::callback<callback_t::name, decltype(new_matcher),
                            typename callback_t::callable_t>{
        std::move(new_matcher), std::forward<C>(c).callable};
};

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
