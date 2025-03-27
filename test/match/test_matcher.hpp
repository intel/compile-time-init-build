#pragma once

#include <match/concepts.hpp>
#include <match/implies.hpp>
#include <match/negate.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>

#include <concepts>
#include <functional>
#include <string_view>

struct test_matcher {
    using is_matcher = void;

    [[nodiscard]] constexpr auto operator()(int i) const -> bool {
        return i == 1;
    }
    [[nodiscard]] constexpr static auto describe() {
        using namespace stdx::literals;
        return "test"_ctst;
    }
    [[nodiscard]] constexpr auto describe_match(int i) const {
        return stdx::ct_format<"{}({} == 1)">((*this)(i) ? 'T' : 'F', i);
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
template <typename RelOp> constexpr auto to_string() {
    using namespace stdx::literals;
    if constexpr (std::same_as<RelOp, std::less<>>) {
        return "<"_ctst;
    } else if constexpr (std::same_as<RelOp, std::less_equal<>>) {
        return "<="_ctst;
    } else if constexpr (std::same_as<RelOp, std::greater<>>) {
        return ">"_ctst;
    } else if constexpr (std::same_as<RelOp, std::greater_equal<>>) {
        return ">="_ctst;
    }
}
} // namespace detail

template <typename RelOp, auto Value> struct rel_matcher {
    using is_matcher = void;

    [[nodiscard]] constexpr auto operator()(int i) const -> bool {
        return RelOp{}(i, Value);
    }
    [[nodiscard]] constexpr static auto describe() {
        using namespace stdx::literals;
        return "test"_ctst;
    }
    [[nodiscard]] constexpr auto describe_match(int i) const {
        return stdx::ct_format<"{}({} {} {})">((*this)(i) ? 'T' : 'F', i,
                                               detail::to_string<RelOp>(),
                                               stdx::ct<Value>());
    }

  private:
    [[nodiscard]] friend constexpr auto tag_invoke(match::negate_t,
                                                   rel_matcher const &) {
        return rel_matcher<decltype(detail::inverse_op<RelOp>()), Value>{};
    }

    template <auto OtherValue>
    [[nodiscard]] friend constexpr auto
    tag_invoke(match::implies_t, rel_matcher, rel_matcher<RelOp, OtherValue>) {
        return RelOp{}(Value, OtherValue);
    }
};

template <auto X, auto Y>
[[nodiscard]] constexpr auto
tag_invoke(match::implies_t, rel_matcher<std::less<>, X> const &,
           rel_matcher<std::less_equal<>, Y> const &) {
    return X <= Y + 1;
}

template <auto X, auto Y>
[[nodiscard]] constexpr auto
tag_invoke(match::implies_t, rel_matcher<std::less_equal<>, X> const &,
           rel_matcher<std::less<>, Y> const &) {
    return X < Y;
}
