#pragma once

#include <match/concepts.hpp>
#include <match/negate.hpp>
#include <sc/format.hpp>
#include <sc/string_constant.hpp>

#include <concepts>
#include <functional>
#include <string_view>

struct test_matcher {
    using is_matcher = void;

    [[nodiscard]] constexpr auto operator()(int i) const -> bool {
        return i == 1;
    }
    [[nodiscard]] constexpr static auto describe() { return "test"_sc; }
    [[nodiscard]] constexpr auto describe_match(int i) const {
        return format("{}({} == 1)"_sc, (*this)(i) ? 'T' : 'F', i);
    }
};
static_assert(match::matcher<test_matcher>);
static_assert(match::matcher_for<test_matcher, int>);

template <auto> struct test_m : test_matcher {};
static_assert(match::matcher<test_m<0>>);
static_assert(match::matcher_for<test_m<0>, int>);

namespace detail {
template <typename RelOp> constexpr auto inverse_op() {
    if constexpr (std::same_as<RelOp, std::less<>>) {
        return std::greater_equal{};
    } else if constexpr (std::same_as<RelOp, std::less_equal<>>) {
        return std::greater{};
    } else if constexpr (std::same_as<RelOp, std::greater<>>) {
        return std::less_equal{};
    } else if constexpr (std::same_as<RelOp, std::greater_equal<>>) {
        return std::less{};
    }
}
template <typename RelOp> constexpr auto to_string() -> std::string_view {
    if constexpr (std::same_as<RelOp, std::less<>>) {
        return "<";
    } else if constexpr (std::same_as<RelOp, std::less_equal<>>) {
        return "<=";
    } else if constexpr (std::same_as<RelOp, std::greater<>>) {
        return ">";
    } else if constexpr (std::same_as<RelOp, std::greater_equal<>>) {
        return ">=";
    }
}
} // namespace detail

template <typename RelOp, auto Value> struct rel_matcher {
    using is_matcher = void;

    [[nodiscard]] constexpr auto operator()(int i) const -> bool {
        return RelOp{}(i, Value);
    }
    [[nodiscard]] constexpr static auto describe() { return "test"_sc; }
    [[nodiscard]] constexpr auto describe_match(int i) const {
        return format("{}({} {} {})"_sc, (*this)(i) ? 'T' : 'F', i,
                      detail::to_string<RelOp>(), Value);
    }

  private:
    [[nodiscard]] friend constexpr auto tag_invoke(match::negate_t,
                                                   rel_matcher const &) {
        return rel_matcher<decltype(detail::inverse_op<RelOp>()), Value>{};
    }
};
