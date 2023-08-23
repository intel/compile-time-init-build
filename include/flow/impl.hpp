#pragma once

#include <container/vector.hpp>
#include <flow/common.hpp>
#include <flow/milestone.hpp>
#include <log/log.hpp>

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
template <typename Name, std::size_t NumSteps> class impl : public interface {
  private:
    constexpr static bool loggingEnabled = not std::is_void_v<Name>;

    constexpr static auto capacity = [] {
        if constexpr (loggingEnabled) {
            return NumSteps * 2;
        } else {
            return NumSteps;
        }
    }();

    cib::vector<FunctionPtr, capacity> functionPtrs{};
    build_status buildStatus;

  public:
    constexpr static bool active = capacity > 0;

    /**
     * Create a new flow::impl of Milestones.
     *
     * Do not call this constructor directly, use flow::builder instead.
     *
     * @param newMilestones
     *      Array of Milestones to execute in the flow.
     *
     * @param buildStatus
     *      flow::builder will report whether the flow::impl can be built
     * successfully, or if there was some other error while building the
     * flow::impl. This can be used along with static_assert to ensure the flow
     * is built correctly.
     *
     * @see flow::builder
     */
    constexpr impl(std::span<milestone_base const> newMilestones,
                   build_status newBuildStatus)
        : buildStatus(newBuildStatus) {
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
        if constexpr (loggingEnabled) {
            CIB_TRACE("flow.start({})", Name{});
        }

        for (auto const func : functionPtrs) {
            func();
        }

        if constexpr (loggingEnabled) {
            CIB_TRACE("flow.end({})", Name{});
        }
    }

    /**
     * @return
     *      Error status of the flow::impl building process.
     */
    [[nodiscard]] constexpr auto getBuildStatus() const -> build_status {
        return buildStatus;
    }
};
} // namespace flow
