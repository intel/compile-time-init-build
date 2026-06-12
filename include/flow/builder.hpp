#pragma once

#include <flow/dsl/walk.hpp>
#include <flow/graph_builder.hpp>
#include <flow/log.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>

#include <utility>

namespace flow {
template <typename Renderer, flow::dsl::subgraph... Fragments>
class builder_for {
  public:
    using interface_t = typename Renderer::interface_t;
    constexpr static auto name = Renderer::name;

    template <flow::dsl::subgraph... Ns>
    [[nodiscard]] constexpr auto add(Ns &&...ns) {
        return fragments.apply([&](auto &...frags) {
            return builder_for<Renderer, Fragments...,
                               stdx::remove_cvref_t<Ns>...>{
                {frags..., std::forward<Ns>(ns)...}};
        });
    }

    template <typename BuilderValue, typename Nexus>
    [[nodiscard]] constexpr static auto build() {
        return Renderer::template render<BuilderValue, Nexus>();
    }

    using is_builder = void;
    stdx::tuple<Fragments...> fragments;
};

template <stdx::ct_string Name = "", typename LogPolicy = log_policy_t<Name>>
using builder = builder_for<graph_builder<Name, LogPolicy>>;
} // namespace flow

namespace flow::dsl::detail {
// Builders aggregate parts. Just expand all of the parts.
template <typename R, typename... Fs>
struct initials_of<flow::builder_for<R, Fs...>> {
    using type = boost::mp11::mp_append<initials_of_t<Fs>...>;
};

template <typename R, typename... Fs>
struct finals_of<flow::builder_for<R, Fs...>> {
    using type = boost::mp11::mp_append<finals_of_t<Fs>...>;
};

template <typename R, typename... Fs>
struct all_mentioned_of<flow::builder_for<R, Fs...>> {
    using type = boost::mp11::mp_append<all_mentioned_of_t<Fs>...>;
};

template <typename R, typename... Fs>
struct nodes_of<flow::builder_for<R, Fs...>> {
    using type = boost::mp11::mp_append<nodes_of_t<Fs>...>;
};

template <typename R, typename... Fs>
struct edges_of<flow::builder_for<R, Fs...>> {
    using type = boost::mp11::mp_append<edges_of_t<Fs>...>;
};
} // namespace flow::dsl::detail
