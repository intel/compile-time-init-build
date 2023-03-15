#pragma once

#include <cib/builder_meta.hpp>
#include <cib/tuple.hpp>
#include <container/Vector.hpp>
#include <log/log.hpp>
#include <msg/callback.hpp>
#include <msg/handler_interface.hpp>

#include <array>
#include <cstddef>
#include <type_traits>

namespace msg {

template <typename CallbacksT, typename BaseMsgT,
          typename... ExtraCallbackArgsT>
struct handler : handler_interface<BaseMsgT, ExtraCallbackArgsT...> {
    CallbacksT callbacks{};

    constexpr explicit handler(CallbacksT new_callbacks)
        : callbacks{new_callbacks} {}

    auto is_match(BaseMsgT const &msg) const -> bool final {
        return callbacks.fold_left(false, [&](bool state, auto &callback) {
            return state || callback.is_match(msg);
        });
    }

    void handle(BaseMsgT const &msg, ExtraCallbackArgsT... args) const final {
        bool const found_valid_callback = cib::any_of(
            [&](auto &callback) { return callback.handle(msg, args...); },
            callbacks);
        if (!found_valid_callback) {
            CIB_ERROR("None of the registered callbacks claimed this message:");
            cib::for_each([&](auto &callback) { callback.log_mismatch(msg); },
                          callbacks);
        }
    }
};

} // namespace msg
