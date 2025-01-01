#pragma once

#include <flow/common.hpp>
#include <flow/graph_builder.hpp>
#include <flow/impl.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/panic.hpp>

namespace flow {
template <stdx::ct_string Name = "">
using builder = graph<Name, graph_builder<Name, impl>>;

template <stdx::ct_string Name = ""> struct service {
    using builder_t = builder<Name>;
    using interface_t = FunctionPtr;

    CONSTEVAL static auto uninitialized() -> interface_t {
        return [] {
            using namespace stdx::literals;
            stdx::panic<"Attempting to run flow ("_cts + Name +
                        ") before it is initialized"_cts>();
        };
    }
};
} // namespace flow
