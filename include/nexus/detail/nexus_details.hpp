#pragma once

#include <nexus/detail/exports.hpp>
#include <nexus/service.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/static_assert.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>

#include <boost/mp11/algorithm.hpp>

#include <type_traits>

namespace cib {
template <typename T> using extract_service_tag = typename T::Service;

namespace detail {
template <typename T, stdx::ct_string Name>
concept name_matches = T::name == Name;

template <stdx::ct_string Name> struct matching_name {
    template <typename T> using fn = std::bool_constant<name_matches<T, Name>>;
};

template <typename Exports, typename T>
constexpr auto locate_service_by_type() {
    using Idx = boost::mp11::mp_find<Exports, typename T::service_type>;
    if constexpr (Idx::value == stdx::tuple_size_v<Exports>) {
        if constexpr (requires { T::name; }) {
            STATIC_ASSERT(
                false, "Trying to extend a service ({}) that is not exported",
                T::name);
        } else {
            STATIC_ASSERT(
                false, "Trying to extend a service ({}) that is not exported",
                T);
        }
    } else {
        return stdx::type_identity<boost::mp11::mp_at<Exports, Idx>>{};
    }
}

template <typename Exports, typename T>
constexpr auto locate_service_by_name() {
    using Idx = boost::mp11::mp_find_if_q<Exports, matching_name<T::name>>;
    if constexpr (Idx::value == stdx::tuple_size_v<Exports>) {
        STATIC_ASSERT(false,
                      "Trying to extend a service ({}) that is not exported",
                      T::name);
    } else {
        return stdx::type_identity<boost::mp11::mp_at<Exports, Idx>>{};
    }
}

template <typename Exports, typename T> constexpr auto locate_service() {
    if constexpr (requires { typename T::service_type; }) {
        return locate_service_by_type<Exports, T>();
    } else {
        STATIC_ASSERT(
            requires { T::name; },
            "Can't locate a service ({}) that has no service_type and no name",
            T);
        return locate_service_by_name<Exports, T>();
    }
}

template <typename Exports, typename T>
using locate_service_t =
    decltype(locate_service<Exports, std::remove_cvref_t<T>>())::type;

template <typename Exports> struct get_service {
    template <typename T> using fn = locate_service_t<Exports, T>;
};
} // namespace detail

template <typename Config>
constexpr static auto initialized_builders = transform<extract_service_tag>(
    []<typename Ext>(Ext extensions) {
        using exports_tuple = decltype(Config::config.exports_tuple());
        using svc = stdx::tuple_element_t<0, Ext>;
        using service = detail::locate_service_t<exports_tuple, svc>;

        constexpr auto initial_builder = builder_t<service>{};

        auto built_service = extensions.fold_right(
            initial_builder, [](auto extension, auto outer_builder) {
                return extension.args_tuple.apply(
                    [&](auto... args) { return outer_builder.add(args...); });
            });

        return detail::service_entry<service, decltype(built_service)>{
            built_service};
    },
    stdx::gather_by<detail::get_service<
        decltype(Config::config.exports_tuple())>::template fn>(
        Config::config.extends_tuple()));

template <typename Config, typename Tag> struct initialized {
    constexpr static auto value =
        initialized_builders<Config>.get(stdx::tag<Tag>).builder;
};
} // namespace cib
