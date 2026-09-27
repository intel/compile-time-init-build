#pragma once

#include <match/concepts.hpp>
#include <match/negate.hpp>

#include <stdx/ct_string.hpp>

// NOTE: the implication overloads in this file are crafted to be high priority,
// to avoid ambiguity. Hence always_t and never_t define friend overloads that
// take "greedy" unconstrained forwarding references, and a specific overload is
// provided for F => T.

namespace match {
struct always_t {
    using is_matcher = void;

    [[nodiscard]] constexpr auto operator()(auto const &) const -> bool {
        return true;
    }
    [[nodiscard]] constexpr static auto describe() {
        using namespace stdx::literals;
        return "true"_ctst;
    }
    [[nodiscard]] constexpr static auto describe_match(auto const &) {
        return describe();
    }

    constexpr auto implied_by(auto &&) const -> bool { return true; }
};

struct never_t {
    using is_matcher = void;

    [[nodiscard]] constexpr auto operator()(auto const &) const -> bool {
        return false;
    }
    [[nodiscard]] constexpr static auto describe() {
        using namespace stdx::literals;
        return "false"_ctst;
    }
    [[nodiscard]] constexpr static auto describe_match(auto const &) {
        return describe();
    }

    constexpr auto implies(auto &&) const -> bool { return true; }

  private:
    [[nodiscard]] friend constexpr auto tag_invoke(negate_t, never_t)
        -> always_t {
        return {};
    }
};

[[nodiscard]] constexpr auto tag_invoke(negate_t, always_t) -> never_t {
    return {};
}

constexpr always_t always{};
constexpr never_t never{};
} // namespace match
