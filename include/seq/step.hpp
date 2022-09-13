#pragma once


#include <seq/detail/step_impl.hpp>
#include <flow/detail/dependency.hpp>
#include <flow/detail/parallel.hpp>


namespace seq {
    using log_func_ptr = void (*)();

    class step_base {
    private:
        func_ptr _forward_ptr;
        func_ptr _backward_ptr;
        log_func_ptr log_name;

        #if defined(__GNUC__) && __GNUC__ < 12
            uint64_t hash;
        #endif

        template</*typename Name,*/ int NumSteps>
        friend class impl;

    public:
        template<typename Name>
        constexpr step_base(
            Name name,
            func_ptr forward_ptr,
            func_ptr backward_ptr
        )
            : _forward_ptr{forward_ptr}
            , _backward_ptr{backward_ptr}
            , log_name{[](){TRACE("seq.step({})", Name{});}}

            #if defined(__GNUC__) && __GNUC__ < 12
                , hash{name.hash()}
            #endif
        {}

        constexpr step_base()
            : _forward_ptr{nullptr}
            , _backward_ptr{nullptr}
            , log_name{nullptr}

            #if defined(__GNUC__) && __GNUC__ < 12
                , hash{0u}
            #endif
        {}

        constexpr step_base(step_base const & rhs) = default;
        constexpr step_base & operator=(step_base const & rhs) = default;
        constexpr step_base(step_base && rhs) = default;
        constexpr step_base & operator=(step_base && rhs) = default;

        [[nodiscard]] constexpr bool operator==(step_base const & rhs) const {
            #if defined(__GNUC__) && __GNUC__ < 12
                return this->hash == rhs.hash;
            #elif
                return
                    (this->_forward_ptr) == (rhs._forward_ptr) &&
                    (this->_backward_ptr) == (rhs._backward_ptr) &&
                    (this->log_name) == (rhs.log_name);
            #endif
        }

        constexpr void forward() const {
            _forward_ptr();
        }

        constexpr void backward() const {
            _backward_ptr();
        }
    };

    template<
        typename LhsT,
        typename RhsT>
    [[nodiscard]] constexpr auto operator>>(
        LhsT const & lhs,
        RhsT const & rhs
    ) {
        return flow::detail::dependency<step_base, LhsT, RhsT>{lhs, rhs};
    }

    template<
        typename LhsT,
        typename RhsT>
    [[nodiscard]] constexpr auto operator&&(
        LhsT const & lhs,
        RhsT const & rhs
    ) {
        return flow::detail::parallel<step_base, LhsT, RhsT>{lhs, rhs};
    }

    /**
     * @param f
     *      The function pointer to execute.
     *
     * @return
     *      New action that will execute the given function pointer, f.
     */
    template<typename NameType>
    [[nodiscard]] constexpr auto step(NameType name, func_ptr forward, func_ptr backward) {
        return step_base{name, forward, backward};
    }
}
