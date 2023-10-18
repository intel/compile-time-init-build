#pragma once

#include <cib/builder_meta.hpp>
#include <msg/handler_interface.hpp>
#include <msg/rle_indexed_builder.hpp>

#include <stdx/tuple.hpp>

namespace msg {
template <typename IndexSpec, typename MsgBaseT, typename... ExtraCallbackArgsT>
struct rle_indexed_service
    : cib::builder_meta<
          rle_indexed_builder<IndexSpec, stdx::tuple<>, MsgBaseT,
                              ExtraCallbackArgsT...>,
          handler_interface<MsgBaseT, ExtraCallbackArgsT...> const *> {};
} // namespace msg
