#pragma once

#include <stdx/ct_string.hpp>

template <stdx::ct_string Name, typename... Args>
extern auto cib_func(Args...) -> void;

namespace cib {
template <stdx::ct_string Name, typename... Args>
constexpr auto func_decl =
    [](Args... args) -> void { cib_func<Name>(args...); };
}
