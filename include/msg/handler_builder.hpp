#pragma once

#include <cib/builder_meta.hpp>
#include <cib/tuple.hpp>
#include <container/Vector.hpp>
#include <log/log.hpp>
#include <msg/handler.hpp>
#include <msg/message.hpp>

#include <array>
#include <cstddef>
#include <type_traits>

namespace msg {
template <typename CallbacksT, typename BaseMsgT,
          typename... ExtraCallbackArgsT>
struct handler_builder {
    CallbacksT callbacks;

    template <typename T> [[nodiscard]] constexpr auto add(T callback) {
        auto new_callbacks =
            cib::tuple_cat(callbacks, cib::make_tuple(callback));

        using new_callbacks_t = decltype(new_callbacks);

        return handler_builder<new_callbacks_t, BaseMsgT,
                               ExtraCallbackArgsT...>{new_callbacks};
    }

    template <typename BuilderValue> static constexpr auto build() {
        return handler<CallbacksT, BaseMsgT, ExtraCallbackArgsT...>{
            BuilderValue::value.callbacks};
    }
};

} // namespace msg
