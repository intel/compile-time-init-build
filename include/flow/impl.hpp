#pragma once

#include <flow/common.hpp>
#include <flow/log.hpp>
#include <flow/step.hpp>
#include <log/log.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/cx_vector.hpp>
#include <stdx/span.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace flow {
namespace detail {
template <typename CTNode> constexpr auto run_func() -> void {
    if (CTNode::condition) {
        typename CTNode::func_t{}();
    }
}

template <typename Flow, typename CTNode> constexpr auto log_func() -> void {
    if (CTNode::condition) {
        using log_spec_t = decltype(get_log_spec<CTNode, Flow>());
        CIB_LOG(typename log_spec_t::flavor, log_spec_t::level, "flow.{}({})",
                typename CTNode::type_t{}, typename CTNode::name_t{});
    }
}
} // namespace detail

struct rt_node {
    FunctionPtr run{};
    FunctionPtr log_name{};

  private:
    friend constexpr auto operator==(rt_node const &,
                                     rt_node const &) -> bool = default;
};

/**
 * flow::impl is a constant representation of a series of Milestones and actions
 * to be executed in a specific order.
 *
 * flow::builder allows multiple independent components to collaboratively
 * specify a flow::impl. Use flow::builder to create Flows. Independent
 * components can then add their own actions and milestones to a flow::impl
 * relative to other actions and milestones.
 *
 * @tparam Name
 *      Name of flow as a compile-time string.
 *
 * @tparam NumSteps
 *      The number of Milestones this flow::impl represents.
 *
 * @see flow::builder
 */
template <stdx::ct_string Name, std::size_t NumSteps> class impl {
  private:
    constexpr static bool loggingEnabled = not Name.empty();

    constexpr static auto capacity = [] {
        if constexpr (loggingEnabled) {
            return NumSteps * 2;
        } else {
            return NumSteps;
        }
    }();

  public:
    stdx::cx_vector<FunctionPtr, capacity> functionPtrs{};

    constexpr static bool active = capacity > 0;
    constexpr static auto name = Name;

    using node_t = rt_node;

    template <typename CTNode>
    constexpr static auto create_node(CTNode) -> node_t {
        constexpr auto rf = detail::run_func<CTNode>;
        constexpr auto lf = detail::log_func<log_spec_id_t<Name>, CTNode>;
        return node_t{rf, lf};
    }

    /**
     * Create a new flow::impl of Milestones.
     *
     * Do not call this constructor directly, use flow::builder instead.
     *
     * @param newMilestones
     *      Array of Milestones to execute in the flow.
     *
     * @see flow::builder
     */
    constexpr explicit(true) impl(stdx::span<node_t const, NumSteps> steps) {
        if constexpr (loggingEnabled) {
            for (auto const &step : steps) {
                functionPtrs.push_back(step.log_name);
                functionPtrs.push_back(step.run);
            }
        } else {
            std::transform(std::cbegin(steps), std::cend(steps),
                           std::back_inserter(functionPtrs),
                           [](auto const &step) { return step.run; });
        }
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
            CIB_LOG(typename log_spec_t::flavor, log_spec_t::level,
                    "flow.start({})", name);
        }

        (FuncPtrs(), ...);

        if constexpr (loggingEnabled) {
            using log_spec_t = decltype(get_log_spec<inlined_func_list>());
            CIB_LOG(typename log_spec_t::flavor, log_spec_t::level,
                    "flow.end({})", name);
        }
    }
};
} // namespace detail

} // namespace flow
