#pragma once

#include <flow/builder.hpp>
#include <flow/log.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/panic.hpp>

namespace flow {
template <typename Builder> struct service_for {
    using builder_t = Builder;
    using interface_t = typename builder_t::interface_t;

    CONSTEVAL static auto uninitialized() -> interface_t {
        return [] {
            using namespace stdx::literals;
            stdx::panic<"Attempting to run flow ("_cts + builder_t::name +
                        ") before it is initialized"_cts>();
        };
    }
};

template <stdx::ct_string Name = "", typename LogPolicy = log_policy_t<Name>>
struct service : service_for<builder<Name, LogPolicy>> {};
} // namespace flow
