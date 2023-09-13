#pragma once

#include <cib/builder_meta.hpp>
#include <msg/handler_builder.hpp>
#include <msg/handler_interface.hpp>

#include <stdx/tuple.hpp>

namespace msg {
template <typename MsgBaseT, typename... ExtraCallbackArgsT>
struct service
    : cib::builder_meta<
          handler_builder<stdx::tuple<>, MsgBaseT, ExtraCallbackArgsT...>,
          handler_interface<MsgBaseT, ExtraCallbackArgsT...> const *> {};
} // namespace msg
