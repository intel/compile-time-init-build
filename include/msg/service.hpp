#pragma once

#include <msg/handler_builder.hpp>
#include <msg/handler_interface.hpp>

#include <stdx/compiler.hpp>

namespace msg {
template <typename MsgBase, typename... ExtraCallbackArgs> struct service {
    using builder_t =
        handler_builder<stdx::tuple<>, MsgBase, ExtraCallbackArgs...>;
    using interface_t =
        handler_interface<MsgBase, ExtraCallbackArgs...> const *;

    constexpr static auto uninitialized_v =
        uninitialized_handler_t<MsgBase, ExtraCallbackArgs...>{};
    CONSTEVAL static auto uninitialized() -> interface_t {
        return &uninitialized_v;
    }
};
} // namespace msg
