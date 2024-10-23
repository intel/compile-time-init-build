#pragma once

#include <match/concepts.hpp>
#include <sc/string_constant.hpp>

#include <stdx/concepts.hpp>
#include <stdx/ct_string.hpp>

#include <type_traits>
#include <utility>

namespace match {
template <stdx::ct_string Name, stdx::callable P> struct predicate_t {
    using is_matcher = void;

    constexpr static P pred{};

    constexpr predicate_t() = default;
    constexpr explicit(true) predicate_t(P) {}

    [[nodiscard]] constexpr auto
    operator()(auto const &event) const -> decltype(pred(event)) {
        return pred(event);
    }

    [[nodiscard]] constexpr static auto describe() {
        return stdx::ct_string_to_type<Name, sc::string_constant>();
    }

    [[nodiscard]] constexpr static auto describe_match(auto const &) {
        return describe();
    }
};

template <stdx::ct_string Name = "<predicate>", stdx::callable P>
constexpr auto predicate(P &&p) {
    return predicate_t<Name, std::remove_cvref_t<P>>{std::forward<P>(p)};
}
} // namespace match
