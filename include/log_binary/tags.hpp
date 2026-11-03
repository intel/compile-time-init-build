#pragma once

#include <log/env.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/type_traits.hpp>

#include <utility>

namespace logging {
namespace detail {
struct no_tag_t {};
template <auto Name, auto Value> struct tag_t {};
} // namespace detail

template <stdx::ct_string Name, stdx::ct_string Value>
using tag_t = detail::tag_t<stdx::shrink<Name>(), stdx::shrink<Value>()>;

[[maybe_unused]] constexpr inline struct get_tag_t {
    template <typename T>
        requires true // more constrained
    consteval auto operator()(T &&t) const
        noexcept(noexcept(std::forward<T>(t).query(std::declval<get_tag_t>())))
            -> decltype(std::forward<T>(t).query(*this)) {
        return std::forward<T>(t).query(*this);
    }

    consteval auto operator()(auto &&) const { return detail::no_tag_t{}; }
} get_tag;
} // namespace logging

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CIB_LOG_TAG(S) CIB_LOG_ENV(logging::get_tag, S)
