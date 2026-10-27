#pragma once

#include <flow/log.hpp>
#include <log/log.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/span.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/utility.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <utility>

namespace flow {
namespace detail {
template <auto FlowName, log_policy LogPolicy, typename CTNode,
          typename Nexus = void>
constexpr static auto run_func = []() -> void {
    if (CTNode::condition) {
        LogPolicy::template log<decltype(get_log_env<CTNode, FlowName>())>(
            __FILE__, __LINE__,
            stdx::ct_format<"flow.{}({})">(stdx::ct<CTNode::ct_type>(),
                                           stdx::constant_name_of_v<CTNode>));
        auto f = typename CTNode::func_t{};
        if constexpr (requires { std::move(f).template operator()<Nexus>(); }) {
            std::move(f).template operator()<Nexus>();
        } else {
            std::move(f)();
        }
    }
};

template <auto Name, log_policy LogPolicy, auto... FuncPtrs>
struct inlined_func_list : stdx::with_shrunk_name<Name> {
    constexpr static auto active = sizeof...(FuncPtrs) > 0;

    __attribute__((flatten, always_inline)) auto operator()() const -> void {
        LogPolicy::template log<decltype(get_log_env<inlined_func_list>())>(
            __FILE__, __LINE__,
            stdx::ct_format<"flow.start({})">(stdx::ct<Name>()));

        (FuncPtrs(), ...);

        LogPolicy::template log<decltype(get_log_env<inlined_func_list>())>(
            __FILE__, __LINE__,
            stdx::ct_format<"flow.end({})">(stdx::ct<Name>()));
    }
};

template <auto Name, log_policy LogPolicy, std::size_t NumSteps>
struct func_list {
    using fp_t = auto (*)() -> void;
    using node_t = std::pair<fp_t, std::string_view>;
    std::array<node_t, NumSteps> nodes{};

    template <typename Nexus, typename CTNode>
    constexpr static auto create_node(CTNode) -> node_t {
        constexpr auto fp = run_func<Name, LogPolicy, CTNode, Nexus>;
        constexpr auto name = std::string_view{stdx::name_of_v<CTNode>};
        return {fp, name};
    }

    constexpr explicit(true)
        func_list(stdx::span<node_t const, NumSteps> steps) {
        std::copy(std::cbegin(steps), std::cend(steps), std::begin(nodes));
    }

    template <fp_t... Fs>
    using finalized_t = inlined_func_list<Name, LogPolicy, Fs...>;
};
} // namespace detail

template <stdx::ct_string Name, log_policy LogPolicy, std::size_t NumSteps>
using func_list = detail::func_list<stdx::shrink<Name>(), LogPolicy, NumSteps>;
} // namespace flow
