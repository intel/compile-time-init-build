#pragma once

#include <match/concepts.hpp>
#include <match/negate.hpp>
#include <sc/string_constant.hpp>

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
};

[[nodiscard]] constexpr auto tag_invoke(negate_t, always_t) -> never_t {
    return {};
}

constexpr always_t always{};
constexpr never_t never{};
} // namespace match
