#pragma once

#include <match/concepts.hpp>
#include <match/implies.hpp>
#include <match/negate.hpp>
#include <sc/string_constant.hpp>

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
    [[nodiscard]] constexpr static auto describe() { return "true"_sc; }
    [[nodiscard]] constexpr static auto describe_match(auto const &) {
        return describe();
    }

  private:
    [[nodiscard]] friend constexpr auto tag_invoke(implies_t, auto &&, always_t)
        -> bool {
        return true;
    }
};

struct never_t {
    using is_matcher = void;

    [[nodiscard]] constexpr auto operator()(auto const &) const -> bool {
        return false;
    }
    [[nodiscard]] constexpr static auto describe() { return "false"_sc; }
    [[nodiscard]] constexpr static auto describe_match(auto const &) {
        return describe();
    }

  private:
    [[nodiscard]] friend constexpr auto tag_invoke(negate_t, never_t)
        -> always_t {
        return {};
    }

    [[nodiscard]] friend constexpr auto tag_invoke(implies_t, never_t, auto &&)
        -> bool {
        return true;
    }
};

[[nodiscard]] constexpr auto tag_invoke(negate_t, always_t) -> never_t {
    return {};
}

[[nodiscard]] constexpr auto tag_invoke(implies_t, never_t, always_t) -> bool {
    return true;
}

constexpr always_t always{};
constexpr never_t never{};
} // namespace match
