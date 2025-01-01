#pragma once

#include <cib/builder_meta.hpp>

#include <stdx/compiler.hpp>
#include <stdx/panic.hpp>

#include <array>
#include <concepts>
#include <cstddef>
#include <utility>

namespace callback {
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
 *      List of argument types that must be passed into the callback when it is
 * invoked.
 *
 * @see callback::service
 */
template <int NumFuncs = 0, typename... ArgTypes> struct builder {
    using func_ptr_t = void (*)(ArgTypes...);
    std::array<func_ptr_t, NumFuncs> funcs{};

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
    template <std::convertible_to<func_ptr_t>... Fs>
    [[nodiscard]] constexpr auto add(Fs &&...fs) const {
        builder<NumFuncs + sizeof...(Fs), ArgTypes...> cb;
        auto i = std::size_t{};
        while (i < NumFuncs) {
            cb.funcs[i] = funcs[i];
            ++i;
        }
        ((cb.funcs[i++] = std::forward<Fs>(fs)), ...);
        return cb;
    }

    /**
     * Build and return a function pointer to the implemented callback
     * builder. Used by cib nexus to automatically build an initialized
     * builder.
     *
     * Do not call directly.
     *
     * @tparam BuilderValue
     *      Struct that contains a "static constexpr auto value" field with the
     * initialized builder.
     *
     * @return
     *      Function pointer to the implemented callback service.
     */
    template <typename BuilderValue>
    [[nodiscard]] CONSTEVAL static auto build() {
        return run<BuilderValue>;
    }

  private:
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
    template <typename BuilderValue> static void run(ArgTypes... args) {
        constexpr auto handler_builder = BuilderValue::value;
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            (handler_builder.funcs[Is](args...), ...);
        }(std::make_index_sequence<NumFuncs>{});
    }
};

/**
 * Extend this to create named callback services.
 *
 * Types that extend service can be used as unique names with
 * cib::exports and cib::extend.
 *
 * @tparam ArgTypes
 *      The function arguments that must be passed into the callback
 *      services implementation. any_t function registered with this
 *      callback service must also have a compatible signature.
 *
 * @see cib::exports
 * @see cib::extend
 */
template <typename... ArgTypes> struct service {
    using builder_t = builder<0, ArgTypes...>;
    using interface_t = void (*)(ArgTypes...);

    CONSTEVAL static auto uninitialized() -> interface_t {
        return [](ArgTypes...) {
            stdx::panic<
                "Attempting to run callback before it is initialized">();
        };
    }
};
} // namespace callback
