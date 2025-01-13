#pragma once

#include <log/env.hpp>

#include <stdx/ct_string.hpp>

#include <utility>

namespace logging {
[[maybe_unused]] constexpr inline struct get_module_t {
    template <typename T>
        requires true // more constrained
    CONSTEVAL auto operator()(T &&t) const noexcept(
        noexcept(std::forward<T>(t).query(std::declval<get_module_t>())))
        -> decltype(std::forward<T>(t).query(*this)) {
        return std::forward<T>(t).query(*this);
    }

    CONSTEVAL auto operator()(auto &&) const {
        using namespace stdx::literals;
        return "default"_ctst;
    }
} get_module;
} // namespace logging

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CIB_LOG_MODULE(S) CIB_LOG_ENV(logging::get_module, S)
