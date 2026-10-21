#pragma once

#include <nexus/detail/config_item.hpp>
#include <nexus/service.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/static_assert.hpp>
#include <stdx/tuple.hpp>

namespace cib::detail {
struct export_optional {
    template <typename T> consteval static auto lookup_failed() -> void {}
};

struct export_required {
    template <typename T> consteval static auto lookup_failed() -> void {
        if constexpr (stdx::named<T>) {
            STATIC_ASSERT(
                false, "Trying to extend a service ({}) that is not exported",
                stdx::name_of_v<T>);
        } else {
            STATIC_ASSERT(
                false, "Trying to extend a service ({}) that is not exported",
                T);
        }
    }
};

template <stdx::ct_string Name, typename ExportPolicy = export_required,
          typename... Args>
struct name_extend : stdx::with_name<Name>, config_item {
    using export_policy_t = ExportPolicy;
    stdx::tuple<Args...> args_tuple;

    consteval explicit name_extend(Args const &...args) : args_tuple{args...} {}

    [[nodiscard]] constexpr auto extends_tuple() const
        -> stdx::tuple<name_extend> {
        return {*this};
    }
};

template <typename Service> struct with_name_of {};
template <stdx::named Service>
struct with_name_of<Service> : stdx::with_name<stdx::name_of_v<Service>> {};

template <typename Service, typename ExportPolicy = export_required,
          typename... Args>
struct type_extend : with_name_of<Service>, config_item {
    using export_policy_t = ExportPolicy;
    using service_type = Service;
    stdx::tuple<Args...> args_tuple;

    consteval explicit type_extend(Args const &...args) : args_tuple{args...} {}

    [[nodiscard]] constexpr auto extends_tuple() const
        -> stdx::tuple<type_extend> {
        return {*this};
    }
};
} // namespace cib::detail
