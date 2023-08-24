#pragma once

#include <cib/builder_meta.hpp>
#include <cib/tuple.hpp>
#include <msg/handler_interface.hpp>
#include <msg/indexed_builder.hpp>

namespace msg {
template <typename IndexSpec, typename MsgBaseT, typename... ExtraCallbackArgsT>
struct indexed_service
    : cib::builder_meta<
          indexed_builder<IndexSpec, cib::tuple<>, MsgBaseT,
                          ExtraCallbackArgsT...>,
          handler_interface<MsgBaseT, ExtraCallbackArgsT...> const *> {};
} // namespace msg
