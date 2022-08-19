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

        #if defined(__GNUC__) && __GNUC__ < 12
            uint64_t hash;
        #endif

        template<typename Name, int NumSteps>
        friend class impl;

    public:
        template<typename Name>
        constexpr milestone_base(
            Name name,
            FunctionPtr run_ptr
        )
            : run{run_ptr}
            , log_name{[](){TRACE("flow.milestone({})", Name{});}}

            #if defined(__GNUC__) && __GNUC__ < 12
                , hash{name.hash()}
            #endif
        {}

        constexpr milestone_base()
            : run{nullptr}
            , log_name{nullptr}

            #if defined(__GNUC__) && __GNUC__ < 12
                , hash{0u}
            #endif
        {}

        constexpr milestone_base(milestone_base const & rhs) = default;
        constexpr milestone_base & operator=(milestone_base const & rhs) = default;
        constexpr milestone_base(milestone_base && rhs) = default;
        constexpr milestone_base & operator=(milestone_base && rhs) = default;

        [[nodiscard]] constexpr bool operator==(milestone_base const & rhs) const {
            #if defined(__GNUC__) && __GNUC__ < 12
                return this->hash == rhs.hash;
            #elif
                return
                    (this->run) == (rhs.run) &&
                    (this->log_name) == (rhs.log_name);
            #endif
        }

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
        return detail::dependency{lhs, rhs};
    }

    template<
        typename LhsT,
        typename RhsT>
    [[nodiscard]] constexpr auto operator&&(
        LhsT const & lhs,
        RhsT const & rhs
    ) {
        return detail::parallel{lhs, rhs};
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
        return milestone_base{name, f};
    }

    /**
     * @return
     *      New milestone_base with no associated action.
     */
    template<typename NameType>
    [[nodiscard]] constexpr auto milestone(NameType name) {
        return milestone_base{name, [](){}};
    }
}
