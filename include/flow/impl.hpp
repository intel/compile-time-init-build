#pragma once

#include <flow/build_status.hpp>
#include <flow/milestone.hpp>

#include <array>
#include <cstddef>
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

    std::array<FunctionPtr, capacity> functionPtrs{};
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
    constexpr impl(milestone_base *newMilestones, build_status newBuildStatus)
        : functionPtrs(), buildStatus(newBuildStatus) {
        if constexpr (loggingEnabled) {
            for (auto i = std::size_t{}; i < NumSteps; i++) {
                functionPtrs[(i * 2)] = newMilestones[i].log_name;
                functionPtrs[(i * 2) + 1] = newMilestones[i].run;
            }
        } else {
            for (auto i = std::size_t{}; i < NumSteps; i++) {
                functionPtrs[i] = newMilestones[i].run;
            }
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

template <typename Name> class impl<Name, 0> {
  private:
    build_status buildStatus;

  public:
    constexpr static bool active = false;

    constexpr impl(milestone_base *, build_status newBuildStatus)
        : buildStatus(newBuildStatus) {}

    constexpr void operator()() const {
        // pass
    }

    [[nodiscard]] constexpr auto getBuildStatus() const -> build_status {
        return buildStatus;
    }
};
} // namespace flow
