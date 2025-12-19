#pragma once

#include <stdx/compiler.hpp>
#include <stdx/utility.hpp>

#include <utility>

namespace logging {
[[maybe_unused]] constexpr inline struct get_unit_t {
    template <typename T>
        requires true
    CONSTEVAL auto operator()(T &&t) const
        noexcept(noexcept(std::forward<T>(t).query(std::declval<get_unit_t>())))
            -> decltype(std::forward<T>(t).query(*this)) {
        return std::forward<T>(t).query(*this);
    }

    CONSTEVAL auto operator()(auto &&) const {
        return [] { return 0; };
    }
} get_unit;
} // namespace logging
