#pragma once

#include <log/env.hpp>

#include <utility>

namespace logging {
[[maybe_unused]] constexpr inline struct get_module_id_t {
    template <typename T>
        requires true // more constrained
    CONSTEVAL auto operator()(T &&t) const noexcept(
        noexcept(std::forward<T>(t).query(std::declval<get_module_id_t>())))
        -> decltype(std::forward<T>(t).query(*this)) {
        return std::forward<T>(t).query(*this);
    }

    CONSTEVAL auto operator()(auto &&) const -> int { return -1; }
} get_module_id;
} // namespace logging
