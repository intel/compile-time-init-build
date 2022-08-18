#pragma once


#include <flow/detail/milestone_impl.hpp>
#include <flow/detail/dependency.hpp>
#include <flow/detail/parallel.hpp>


namespace flow {
    using FunctionPtr = void (*)();

    class milestone_base {
    private:
        FunctionPtr run;
        FunctionPtr log_name;

        template<typename Name, int NumSteps>
        friend class impl;

    public:
        constexpr milestone_base(
            FunctionPtr run_ptr,
            FunctionPtr log_name_ptr
        )
            : run{run_ptr}
            , log_name{log_name_ptr}
        {}

        constexpr milestone_base(milestone_base const & rhs) = default;
        constexpr milestone_base & operator=(milestone_base const & rhs) = default;
        constexpr milestone_base(milestone_base && rhs) = default;
        constexpr milestone_base & operator=(milestone_base && rhs) = default;

        constexpr void operator()() const {
            run();
        }
    };

    template<
        typename LhsT,
        typename RhsT>
    [[nodiscard]] constexpr auto operator>>(
        LhsT const & lhs,
        RhsT const & rhs
    ) {
        return detail::dependency{
            detail::convert_milestone_to_ptr(lhs),
            detail::convert_milestone_to_ptr(rhs)};
    }

    template<
        typename LhsT,
        typename RhsT>
    [[nodiscard]] constexpr auto operator&&(
        LhsT const & lhs,
        RhsT const & rhs
    ) {
        return detail::parallel{
            detail::convert_milestone_to_ptr(lhs),
            detail::convert_milestone_to_ptr(rhs)};
    }

    /**
     * @param f
     *      The function pointer to execute.
     *
     * @return
     *      New action that will execute the given function pointer, f.
     */
    template<typename NameType>
    [[nodiscard]] constexpr auto action(NameType name, FunctionPtr f) {
        return milestone_base{f, detail::log_flow_milestone<NameType>};
    }

    /**
     * @return
     *      New milestone_base with no associated action.
     */
    template<typename NameType>
    [[nodiscard]] constexpr auto milestone(NameType name) {
        return milestone_base{[]{}, detail::log_flow_milestone<NameType>};
    }
}
