#pragma once

#include <flow/common.hpp>
#include <flow/log.hpp>
#include <log/env.hpp>
#include <log/level.hpp>
#include <log/log.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/span.hpp>

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
            using log_spec_t =
                decltype(get_log_spec<CTNode, log_spec_id_t<FlowName>>());
            CIB_LOG_ENV(logging::get_level, log_spec_t::level);
            CIB_LOG(typename log_spec_t::flavor, "flow.{}({})",
                    typename CTNode::type_t{}, typename CTNode::name_t{});
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

        constexpr auto name =
            stdx::ct_string_to_type<Name, sc::string_constant>();

        if constexpr (loggingEnabled) {
            using log_spec_t = decltype(get_log_spec<inlined_func_list>());
            CIB_LOG_ENV(logging::get_level, log_spec_t::level);
            CIB_LOG(typename log_spec_t::flavor, "flow.start({})", name);
        }

        (FuncPtrs(), ...);

        if constexpr (loggingEnabled) {
            using log_spec_t = decltype(get_log_spec<inlined_func_list>());
            CIB_LOG_ENV(logging::get_level, log_spec_t::level);
            CIB_LOG(typename log_spec_t::flavor, "flow.end({})", name);
        }
    }
};
} // namespace detail
} // namespace flow
