#pragma once

#include <log/log.hpp>
#include <msg/handler_interface.hpp>

#include <stdx/tuple_algorithms.hpp>

namespace msg {

template <typename Callbacks, typename BaseMsg, typename... ExtraCallbackArgs>
struct handler : handler_interface<BaseMsg, ExtraCallbackArgs...> {
    Callbacks callbacks{};

    constexpr explicit handler(Callbacks new_callbacks)
        : callbacks{new_callbacks} {}

    auto is_match(BaseMsg const &msg) const -> bool final {
        return stdx::any_of(
            [&](auto &callback) { return callback.is_match(msg); }, callbacks);
    }

    auto handle(BaseMsg const &msg, ExtraCallbackArgs... args) const
        -> bool final {
        bool const found_valid_callback = stdx::any_of(
            [&](auto &callback) { return callback.handle(msg, args...); },
            callbacks);
        if (!found_valid_callback) {
            CIB_ERROR("None of the registered callbacks claimed this message:");
            stdx::for_each([&](auto &callback) { callback.log_mismatch(msg); },
                           callbacks);
        }
        return found_valid_callback;
    }
};

} // namespace msg
