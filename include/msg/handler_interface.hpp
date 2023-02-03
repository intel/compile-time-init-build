#pragma once

namespace msg {
template <typename MsgBaseT, typename... ExtraCallbackArgsT>
struct handler_interface {
    virtual auto is_match(MsgBaseT const &msg) const -> bool = 0;

    virtual void handle(MsgBaseT const &msg,
                        ExtraCallbackArgsT... extra_args) const = 0;
};
} // namespace msg
