#pragma once

#include <flow/common.hpp>
#include <flow/log.hpp>
#include <log/log.hpp>

#include <stdx/ct_format.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/span.hpp>
#include <stdx/type_traits.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>

namespace flow {
namespace detail {
template <stdx::ct_string FlowName, typename CTNode>
constexpr auto run_func() -> void {
    if (CTNode::condition) {
        if constexpr (not FlowName.empty()) {
            logging::log<
                decltype(get_log_env<CTNode, log_env_id_t<FlowName>>())>(
                __FILE__, __LINE__,
                stdx::ct_format<"flow.{}({})">(stdx::cts_t<CTNode::ct_type>{},
                                               stdx::cts_t<CTNode::ct_name>{}));
        }
        typename CTNode::func_t{}();
    }
}
} // namespace detail

template <stdx::ct_string Name, std::size_t NumSteps> struct impl {
    using node_t = FunctionPtr;
    std::array<FunctionPtr, NumSteps> functionPtrs{};

    constexpr static auto name = Name;

    template <typename CTNode>
    constexpr static auto create_node(CTNode) -> node_t {
        constexpr auto fp = detail::run_func<Name, CTNode>;
        return fp;
    }

    constexpr explicit(true) impl(stdx::span<node_t const, NumSteps> steps) {
        std::copy(std::cbegin(steps), std::cend(steps),
                  std::begin(functionPtrs));
    }
};

namespace detail {
template <stdx::ct_string Name, auto... FuncPtrs> struct inlined_func_list {
    constexpr static auto active = sizeof...(FuncPtrs) > 0;
    constexpr static auto ct_name = Name;

    __attribute__((flatten, always_inline)) auto operator()() const -> void {
        constexpr static bool loggingEnabled = not Name.empty();

        if constexpr (loggingEnabled) {
            logging::log<decltype(get_log_env<inlined_func_list>())>(
                __FILE__, __LINE__,
                stdx::ct_format<"flow.start({})">(stdx::cts_t<Name>{}));
        }

        (FuncPtrs(), ...);

        if constexpr (loggingEnabled) {
            logging::log<decltype(get_log_env<inlined_func_list>())>(
                __FILE__, __LINE__,
                stdx::ct_format<"flow.end({})">(stdx::cts_t<Name>{}));
        }
    }
};
} // namespace detail
} // namespace flow
