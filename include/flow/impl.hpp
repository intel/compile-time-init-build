#pragma once

#include <flow/common.hpp>
#include <flow/step.hpp>
#include <log/log.hpp>

#include <stdx/cx_vector.hpp>
#include <stdx/span.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace flow {
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

    using node_t = rt_node;
    constexpr static bool active = capacity > 0;

    constexpr static auto name = Name;

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
    constexpr explicit(true)
        impl(stdx::span<node_t const, NumSteps> newMilestones) {
        if constexpr (loggingEnabled) {
            for (auto const &milestone : newMilestones) {
                functionPtrs.push_back(milestone.log_name);
                functionPtrs.push_back(milestone.run);
            }
        } else {
            std::transform(std::cbegin(newMilestones), std::cend(newMilestones),
                           std::back_inserter(functionPtrs),
                           [](auto const &milestone) { return milestone.run; });
        }
    }
};

namespace detail {
template <stdx::ct_string Name, auto... FuncPtrs> struct inlined_func_list {
    constexpr static auto active = sizeof...(FuncPtrs) > 0;

    __attribute__((flatten, always_inline)) auto operator()() const -> void {
        constexpr static bool loggingEnabled = not Name.empty();

        constexpr auto name =
            stdx::ct_string_to_type<Name, sc::string_constant>();

        if constexpr (loggingEnabled) {
            CIB_TRACE("flow.start({})", name);
        }

        (FuncPtrs(), ...);

        if constexpr (loggingEnabled) {
            CIB_TRACE("flow.end({})", name);
        }
    }
};
} // namespace detail

} // namespace flow
