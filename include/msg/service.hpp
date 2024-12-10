#pragma once

#include <msg/handler_builder.hpp>
#include <msg/handler_interface.hpp>

#include <stdx/tuple.hpp>

namespace msg {
template <typename MsgBase, typename... ExtraCallbackArgs> struct service {
    using builder_t =
        handler_builder<stdx::tuple<>, MsgBase, ExtraCallbackArgs...>;
    using interface_t =
        handler_interface<MsgBase, ExtraCallbackArgs...> const *;
};
} // namespace msg
