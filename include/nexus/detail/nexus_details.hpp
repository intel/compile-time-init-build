#pragma once

#include <nexus/detail/exports.hpp>
#include <nexus/service.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>

#include <type_traits>

namespace cib {
template <typename T> using extract_service_tag = typename T::Service;

namespace detail {
template <typename T> constexpr auto locate_service() {
    if constexpr (requires { typename T::service_type; }) {
        return stdx::type_identity<typename T::service_type>{};
    } else {
        return stdx::type_identity<
            std::remove_const_t<decltype(service_locator<T::name>)>>{};
    }
}

template <typename T>
using get_service = decltype(locate_service<std::remove_cvref_t<T>>())::type;

template <typename T>
using get_service_from_tuple =
    get_service<stdx::tuple_element_t<0, std::remove_cvref_t<T>>>;
} // namespace detail

template <typename Config>
constexpr static auto initialized_builders = transform<extract_service_tag>(
    [](auto extensions) {
        using exports_tuple = decltype(Config::config.exports_tuple());
        using service = detail::get_service_from_tuple<decltype(extensions)>;
        static_assert(stdx::contains_type<exports_tuple, service>);

        constexpr auto initial_builder = builder_t<service>{};

        auto built_service = extensions.fold_right(
            initial_builder, [](auto extension, auto outer_builder) {
                return extension.args_tuple.apply(
                    [&](auto... args) { return outer_builder.add(args...); });
            });

        return detail::service_entry<service, decltype(built_service)>{
            built_service};
    },
    stdx::gather_by<detail::get_service>(Config::config.extends_tuple()));

template <typename Config, typename Tag> struct initialized {
    constexpr static auto value =
        initialized_builders<Config>.get(stdx::tag<Tag>).builder;
};
} // namespace cib
