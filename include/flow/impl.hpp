#pragma once


#include <flow/milestone.hpp>
#include <flow/build_status.hpp>

#include <type_traits>


namespace flow {
    /**
     * flow::impl is a constant representation of a series of Milestones and actions to
     * be executed in a specific order.
     *
     * flow::Builder allows multiple independent modules to collaboratively specify
     * a flow::impl. Use flow::Builder to create Flows. Independent modules can then add
     * their own actions and milestones to a flow::impl relative to other actions and
     * milestones.
     *
     * @tparam Name
     *      Name of flow as a compile-time string.
     *
     * @tparam NumSteps
     *      The number of Milestones this flow::impl represents.
     *
     * @see flow::Builder
     */
    template<typename Name, int NumSteps>
    class impl {
    private:
        constexpr static bool loggingEnabled = !std::is_same<Name, void>::value;
        
        constexpr static auto capacity = []{
            if constexpr (loggingEnabled) {
                return NumSteps * 2;
            } else {
                return NumSteps;
            }
        }();

        FunctionPtr functionPtrs[capacity];
        build_status buildStatus;

    public:
        constexpr static bool active = capacity > 0;

        /**
         * Create a new flow::impl of Milestones.
         *
         * Do not call this constructor directly, use flow::Builder instead.
         *
         * @param newMilestones
         *      Array of Milestones to execute in the flow.
         *
         * @param buildStatus
         *      flow::Builder will report whether the flow::impl can be built successfully,
         *      or if there was some other error while building the flow::impl. This can be
         *      used along with static_assert to ensure the flow is built correctly.
         *
         * @see flow::Builder
         */
        constexpr impl(
            milestone_base const * newMilestones[],
            build_status newBuildStatus
        )
            : functionPtrs()
            , buildStatus(newBuildStatus)
        {
            if constexpr (loggingEnabled) {
                for (int i = 0; i < NumSteps; i++) {
                    functionPtrs[(i * 2)] = newMilestones[i]->log_name;
                    functionPtrs[(i * 2) + 1] = newMilestones[i]->run;
                }
            } else {
                for (int i = 0; i < NumSteps; i++) {
                    functionPtrs[i] = newMilestones[i]->run;
                }
            }
        }

        /**
         * Execute the entire flow in order.
         */
        constexpr void operator()() const {
            if constexpr (loggingEnabled) {
                TRACE("flow.start({})", Name{});
            }

            for (auto const func : functionPtrs) {
                func();
            }

            if constexpr(loggingEnabled) {
                TRACE("flow.end({})", Name{});
            }
        }

        /**
         * @return
         *      Error status of the flow::impl building process.
         */
        [[nodiscard]] constexpr build_status getBuildStatus() const {
            return buildStatus;
        }
    };

    template<typename Name>
    class impl<Name, 0> {
    private:
        build_status buildStatus;

    public:
        constexpr static bool active = false;

        constexpr impl(
            milestone_base const * newMilestones[],
            build_status newBuildStatus
        )
            : buildStatus(newBuildStatus)
        {}

        constexpr void operator()() const {
            // pass
        }

        [[nodiscard]] constexpr build_status getBuildStatus() const {
            return buildStatus;
        }
    };
}