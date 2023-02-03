#pragma once

#include <cib/builder_meta.hpp>
#include <cib/tuple.hpp>
#include <msg/handler_builder.hpp>
#include <msg/handler_interface.hpp>

namespace msg {
template <typename MsgBaseT, typename... ExtraCallbackArgsT>
struct service
    : cib::builder_meta<
          handler_builder<cib::tuple<>, MsgBaseT, ExtraCallbackArgsT...>,
          handler_interface<MsgBaseT, ExtraCallbackArgsT...> const *> {};
} // namespace msg
