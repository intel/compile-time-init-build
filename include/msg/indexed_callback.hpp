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

template <typename... Fields>
constexpr auto remove_match_terms = []<typename C>(C &&c) {
    using callback_t = std::remove_cvref_t<C>;
    match::matcher auto new_matcher = remove_terms(
        std::forward<C>(c).matcher, std::type_identity<Fields>{}...);
    return detail::callback<callback_t::name, typename callback_t::msg_t,
                            decltype(new_matcher),
                            typename callback_t::callable_t>{
        std::move(new_matcher), std::forward<C>(c).callable};
};
} // namespace msg
