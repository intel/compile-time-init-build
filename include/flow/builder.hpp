#pragma once

#include <flow/dsl/walk.hpp>
#include <flow/graph_builder.hpp>
#include <flow/log.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>

#include <utility>

namespace flow {
template <typename Renderer, flow::dsl::subgraph... Fragments>
class builder_for {
    template <typename Tag>
    friend constexpr auto tag_invoke(Tag, builder_for const &b) {
        return b.fragments.apply([](auto const &...frags) {
            return stdx::tuple_cat(Tag{}(frags)...);
        });
    }

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

    template <typename BuilderValue>
    [[nodiscard]] constexpr static auto build() {
        return Renderer::template render<BuilderValue>();
    }

    stdx::tuple<Fragments...> fragments;
};

template <stdx::ct_string Name = "", typename LogPolicy = log_policy_t<Name>>
using builder = builder_for<graph_builder<Name, LogPolicy>>;
} // namespace flow
