#pragma once

#include <cib/detail/runtime_conditional.hpp>
#include <flow/subgraph_identity.hpp>
#include <log/log.hpp>
#include <sc/string_constant.hpp>

#include <stdx/ct_string.hpp>

#include <cstdint>

namespace seq {
enum struct status : std::uint8_t { NOT_DONE, DONE };

using func_ptr = auto (*)() -> status;
using log_func_ptr = auto (*)() -> void;

struct rt_step {
    using is_subgraph = void;

    func_ptr forward_ptr{};
    func_ptr backward_ptr{};
    log_func_ptr log_name{};

  private:
    [[nodiscard]] constexpr friend auto operator==(rt_step const &,
                                                   rt_step const &)
        -> bool = default;
};

template <stdx::ct_string Name, bool IsReference = true>
struct ct_step : rt_step {
    using is_subgraph = void;
    using name_t =
        decltype(stdx::ct_string_to_type<Name, sc::string_constant>());

    constexpr static auto identity = flow::subgraph_identity::VALUE;

    constexpr static auto condition = cib::detail::always_condition;

    constexpr static auto ct_name = Name;

    constexpr auto operator*() const -> ct_step<Name, false> {
        return {forward_ptr, backward_ptr, log_name};
    }
};

/**
 * @param forward
 *      The function pointer to execute on a forward step.
 * @param backward
 *      The function pointer to execute on a backward step.
 *
 * @return
 *      Step that will execute the given function pointers.
 */
template <stdx::ct_string Name>
[[nodiscard]] constexpr auto step(func_ptr forward, func_ptr backward) {
    return ct_step<Name>{
        {forward, backward, [] {
             CIB_TRACE("seq.step({})",
                       stdx::ct_string_to_type<Name, sc::string_constant>());
         }}};
}
} // namespace seq
