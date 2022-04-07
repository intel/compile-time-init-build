#include "builder_meta.hpp"
#include "detail/meta.hpp"
#include "detail/compiler.hpp"

#include <array>
#include <type_traits>


#ifndef COMPILE_TIME_INIT_BUILD_CALLBACK_HPP
#define COMPILE_TIME_INIT_BUILD_CALLBACK_HPP


namespace cib {
    /**
     * Extension point/builder for simple callbacks.
     *
     * Modules can add their own callback function to this builder to be executed when
     * the builder is executed with the same function arguments.
     *
     * @tparam Size
     *      Maximum number of callbacks that may be registered.
     *
     * @tparam Args
     *      List of argument types that must be passed into the callback when it is invoked.
     */
    template<int Size = 0, typename... Args>
    struct callback {
    private:
        using func_ptr_t = void(*)(Args...);

        std::array<func_ptr_t, Size> funcs;

        template<typename BuilderValue>
        static void run(Args... args) {
            CIB_CONSTEXPR auto handlerBuilder = BuilderValue::value;
            CIB_CONSTEXPR auto numFuncs = std::integral_constant<int, Size>{};

            detail::for_each(numFuncs, [&](auto i){
                CIB_CONSTEXPR auto func = handlerBuilder.funcs[i];
                func(args...);
            });
        }

    public:
        CIB_CONSTEVAL callback() = default;

        template<typename PrevFuncsT>
        CIB_CONSTEVAL callback(
            PrevFuncsT const & prev_funcs,
            func_ptr_t new_func
        )
            : funcs{}
        {
            for (int i = 0; i < prev_funcs.size(); i++) {
                funcs[i] = prev_funcs[i];
            }

            funcs[Size - 1] = new_func;
        }

        // cib uses "add(...)" to add features to service builders
        CIB_CONSTEVAL auto add(func_ptr_t const & func) const {
            return callback<Size + 1, Args...>{funcs, func};
        }

        /**
         * Build and return a function pointer to the implemented callback builder. Used
         * by cib library to automatically build an initialized builder. Do not call.
         *
         * @tparam BuilderValue
         *      Struct that contains a "static constexpr auto value" field with the initialized
         *      builder.
         *
         * @return
         *      Function pointer to callback builder.
         */
        template<typename BuilderValue>
        [[nodiscard]] CIB_CONSTEVAL static auto build() {
            return run<BuilderValue>;
        }
    };

    template<typename... Args>
    struct callback_meta :
        public cib::builder_meta<
            callback<0, Args...>,
            void(*)(Args...)>
    {};
}


#endif //COMPILE_TIME_INIT_BUILD_CALLBACK_HPP
