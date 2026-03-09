#pragma once

#include <log/env.hpp>

#include <stdx/ct_string.hpp>

#include <utility>

namespace logging {
struct no_tag_t {};

template <stdx::ct_string Name, stdx::ct_string Value> struct tag_t {};

[[maybe_unused]] constexpr inline struct get_tag_t {
    template <typename T>
        requires true // more constrained
    CONSTEVAL auto operator()(T &&t) const
        noexcept(noexcept(std::forward<T>(t).query(std::declval<get_tag_t>())))
            -> decltype(std::forward<T>(t).query(*this)) {
        return std::forward<T>(t).query(*this);
    }

    CONSTEVAL auto operator()(auto &&) const { return no_tag_t{}; }
} get_tag;
} // namespace logging

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CIB_LOG_TAG(S) CIB_LOG_ENV(logging::get_tag, S)
