#include "builder_meta.hpp"
#include "detail/meta.hpp"
#include "detail/compiler.hpp"

#include <array>
#include <type_traits>


#ifndef COMPILE_TIME_INIT_BUILD_CALLBACK_HPP
#define COMPILE_TIME_INIT_BUILD_CALLBACK_HPP


namespace cib {
    /**
     * Builder for simple callbacks.
     *
     * Components can add their own callback function to this builder to be
     * executed when the service is executed with the same function arguments.
     *
     * @tparam NumFuncs
     *      The number of functions currently registered with this builder.
     *
     * @tparam ArgTypes
     *      List of argument types that must be passed into the callback when it is invoked.
     *
     * @see cib::callback_meta
     */
    template<int NumFuncs = 0, typename... ArgTypes>
    struct callback {
    private:
        using func_ptr_t = void(*)(ArgTypes...);

        std::array<func_ptr_t, NumFuncs> funcs;

        /**
         * Runtime implementation of a callback service.
         *
         * Calls each registered function in an undefined order. The order
         * functions are called should not be depended upon and could
         * change from one release to the next.
         *
         * This function will be available from nexus::builder<...> or
         * cib::built<...>.
         *
         * @tparam BuilderValue
         *      A type that contains a constexpr static value field with the
         *      fully initialized callback builder.
         *
         * @param args
         *      The arguments to be passed to every registered function.
         *
         * @see cib::nexus
         * @see cib::built
         */
        template<typename BuilderValue>
        static void run(ArgTypes... args) {
            CIB_CONSTEXPR auto handler_builder = BuilderValue::value;
            CIB_CONSTEXPR auto num_funcs = std::integral_constant<int, NumFuncs>{};

            detail::for_each(num_funcs, [&](auto i){
                CIB_CONSTEXPR auto func = handler_builder.funcs[i];
                func(args...);
            });
        }

    public:
        CIB_CONSTEVAL callback() = default;

        template<typename PrevFuncsType>
        CIB_CONSTEVAL explicit callback(
            PrevFuncsType const & prev_funcs,
            func_ptr_t new_func
        )
            : funcs{}
        {
            for (unsigned int i = 0; i < prev_funcs.size(); i++) {
                funcs[i] = prev_funcs[i];
            }

            funcs[NumFuncs - 1] = new_func;
        }

        /**
         * Add a function to be executed when the callback service is invoked.
         *
         * Do not call this function directly. The library will add functions
         * to service builders based on a project's cib::config and cib::extend
         * declarations.
         *
         * @return
         *      A version of this callback builder with the addition of func.
         *
         * @see cib::extend
         * @see cib::nexus
         */
        [[nodiscard]] CIB_CONSTEVAL auto add(func_ptr_t const & func) const {
            return callback<NumFuncs + 1, ArgTypes...>{funcs, func};
        }

        /**
         * Build and return a function pointer to the implemented callback
         * builder. Used by cib nexus to automatically build an initialized
         * builder.
         *
         * Do not call directly.
         *
         * @tparam BuilderValue
         *      Struct that contains a "static constexpr auto value" field with the initialized
         *      builder.
         *
         * @return
         *      Function pointer to the implemented callback service.
         */
        template<typename BuilderValue>
        [[nodiscard]] CIB_CONSTEVAL static auto build() {
            return run<BuilderValue>;
        }
    };

    /**
     * Extend this to create named callback services.
     *
     * Types that extend callback_meta can be used as unique names with
     * cib::exports and cib::extend.
     *
     * @tparam ArgTypes
     *      The function arguments that must be passed into the callback
     *      services implementation. Any function registered with this
     *      callback service must also have a compatible signature.
     *
     * @see cib::exports
     * @see cib::extend
     */
    template<typename... ArgTypes>
    struct callback_meta :
        public cib::builder_meta<
            callback<0, ArgTypes...>,
            void(*)(ArgTypes...)>
    {};
}


#endif //COMPILE_TIME_INIT_BUILD_CALLBACK_HPP
