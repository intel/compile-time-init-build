#pragma once

#include <msg/handler_interface.hpp>
#include <msg/indexed_builder.hpp>

#include <stdx/compiler.hpp>
#include <stdx/tuple.hpp>

namespace msg {
template <typename IndexSpec, typename MsgBase, typename... ExtraCallbackArgs>
struct indexed_service {
    using builder_t = indexed_builder<IndexSpec, stdx::tuple<>, MsgBase,
                                      ExtraCallbackArgs...>;
    using interface_t =
        handler_interface<MsgBase, ExtraCallbackArgs...> const *;

    constexpr static auto uninitialized_v =
        uninitialized_handler_t<MsgBase, ExtraCallbackArgs...>{};
    CONSTEVAL static auto uninitialized() -> interface_t {
        return &uninitialized_v;
    }
};
} // namespace msg
