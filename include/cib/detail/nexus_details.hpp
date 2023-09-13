#pragma once

#include <cib/detail/exports.hpp>

#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <type_traits>
#include <utility>

namespace cib {
template <typename T> using extract_service_tag = typename T::Service;

template <typename T>
using get_service = typename std::remove_cvref_t<T>::service_type;

template <typename T>
using get_service_from_tuple = typename std::remove_cvref_t<
    decltype(std::declval<T>()[stdx::index<0>])>::service_type;

template <typename Config>
constexpr static auto initialized_builders = transform<extract_service_tag>(
    [](auto extensions) {
        using exports_tuple = decltype(Config::config.exports_tuple());
        using service = get_service_from_tuple<decltype(extensions)>;
        static_assert(stdx::contains_type<exports_tuple, service>);

        constexpr auto initial_builder = extensions[stdx::index<0>].builder;

        auto built_service = extensions.fold_right(
            initial_builder, [](auto extension, auto outer_builder) {
                return extension.args_tuple.apply(
                    [&](auto... args) { return outer_builder.add(args...); });
            });

        return detail::service_entry<service, decltype(built_service)>{
            built_service};
    },
    chunk_by<get_service>(sort<get_service>(Config::config.extends_tuple())));

template <typename Config, typename Tag> struct initialized {
    constexpr static auto value =
        initialized_builders<Config>.get(stdx::tag<Tag>).builder;
};
} // namespace cib
