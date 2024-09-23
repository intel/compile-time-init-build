#pragma once

#include <cib/builder_meta.hpp>
#include <msg/handler_interface.hpp>
#include <msg/indexed_builder.hpp>

#include <stdx/tuple.hpp>

namespace msg {
template <typename IndexSpec, typename MsgBase, typename... ExtraCallbackArgs>
struct indexed_service
    : cib::builder_meta<
          indexed_builder<IndexSpec, stdx::tuple<>, MsgBase,
                          ExtraCallbackArgs...>,
          handler_interface<MsgBase, ExtraCallbackArgs...> const *> {};
} // namespace msg
