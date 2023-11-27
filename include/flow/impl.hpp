#pragma once

#include <flow/common.hpp>
#include <flow/milestone.hpp>
#include <log/log.hpp>

#include <stdx/cx_vector.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <span>
#include <type_traits>

namespace flow {
// NOLINTNEXTLINE(cppcoreguidelines-virtual-class-destructor)
struct interface {
    virtual auto operator()() const -> void {}
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
template <stdx::ct_string Name, std::size_t NumSteps>
class impl : public interface {
  private:
    constexpr static bool loggingEnabled = not Name.empty();

    constexpr static auto capacity = [] {
        if constexpr (loggingEnabled) {
            return NumSteps * 2;
        } else {
            return NumSteps;
        }
    }();

    stdx::cx_vector<FunctionPtr, capacity> functionPtrs{};

  public:
    using node_t = rt_node;
    constexpr static bool active = capacity > 0;

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
    constexpr explicit(true) impl(std::span<node_t const> newMilestones) {
        CIB_ASSERT(NumSteps >= std::size(newMilestones));
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

    /**
     * Execute the entire flow in order.
     */
    auto operator()() const -> void final {
        constexpr auto name =
            stdx::ct_string_to_type<Name, sc::string_constant>();
        if constexpr (loggingEnabled) {
            CIB_TRACE("flow.start({})", name);
        }

        for (auto const func : functionPtrs) {
            func();
        }

        if constexpr (loggingEnabled) {
            CIB_TRACE("flow.end({})", name);
        }
    }
};
} // namespace flow
