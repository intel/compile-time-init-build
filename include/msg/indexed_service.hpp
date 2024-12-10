#pragma once

#include <msg/handler_interface.hpp>
#include <msg/indexed_builder.hpp>

#include <stdx/tuple.hpp>

namespace msg {
template <typename IndexSpec, typename MsgBase, typename... ExtraCallbackArgs>
struct indexed_service {
    using builder_t = indexed_builder<IndexSpec, stdx::tuple<>, MsgBase,
                                      ExtraCallbackArgs...>;
    using interface_t =
        handler_interface<MsgBase, ExtraCallbackArgs...> const *;
};
} // namespace msg
