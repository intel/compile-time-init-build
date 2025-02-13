#pragma once

#include <stdx/compiler.hpp>
#include <stdx/utility.hpp>

#include <utility>

namespace logging {
struct default_flavor_t;

[[maybe_unused]] constexpr inline struct get_flavor_t {
    template <typename T>
        requires true
    CONSTEVAL auto operator()(T &&t) const noexcept(
        noexcept(std::forward<T>(t).query(std::declval<get_flavor_t>())))
        -> decltype(std::forward<T>(t).query(*this)) {
        return std::forward<T>(t).query(*this);
    }

    CONSTEVAL auto operator()(auto &&) const {
        return stdx::ct<stdx::type_identity<default_flavor_t>{}>();
    }
} get_flavor;
} // namespace logging
