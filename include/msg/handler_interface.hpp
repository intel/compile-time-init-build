#pragma once

namespace msg {
template <typename MsgBase, typename... ExtraCallbackArgs>
struct handler_interface {
    virtual auto is_match(MsgBase const &msg) const -> bool = 0;

    virtual auto handle(MsgBase const &msg,
                        ExtraCallbackArgs... extra_args) const -> bool = 0;
};
} // namespace msg
