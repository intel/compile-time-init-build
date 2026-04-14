#pragma once

#include <nexus/detail/config_item.hpp>
#include <nexus/service.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>

namespace cib::detail {
template <stdx::ct_string Name, typename... Args>
struct name_extend : config_item {
    constexpr static auto name = Name;
    stdx::tuple<Args...> args_tuple;

    consteval explicit name_extend(Args const &...args) : args_tuple{args...} {}

    [[nodiscard]] constexpr auto extends_tuple() const
        -> stdx::tuple<name_extend> {
        return {*this};
    }
};

template <typename Service, typename... Args> struct type_extend : config_item {
    using service_type = Service;
    stdx::tuple<Args...> args_tuple;

    consteval explicit type_extend(Args const &...args) : args_tuple{args...} {}

    [[nodiscard]] constexpr auto extends_tuple() const
        -> stdx::tuple<type_extend> {
        return {*this};
    }
};
} // namespace cib::detail
