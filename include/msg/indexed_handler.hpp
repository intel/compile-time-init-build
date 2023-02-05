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

template<typename... IndicesT>
struct indices : IndicesT... {
    template<typename T>
    constexpr auto operator()(T const & data) const {
        return (this->IndicesT::operator()(data) & ...);
    }
};

using callback_func_t = void(*)(BaseMsgT const &, ExtraCallbackArgsT... args);

template<
    typename IndexT,
    typename CallbacksT,
    typename BaseMsgT,
    typename... ExtraCallbackArgsT>
struct indexed_handler {
    IndexT index{};
    CallbacksT callback_entries{};

    constexpr explicit indexed_handler(
        IndexT new_index,
        CallbacksT new_callbacks
    )
        : index{new_index}
        , callbacks{new_callbacks}
    {}

    void handle(BaseMsgT const &msg, ExtraCallbackArgsT... args) const final {
        auto const callback_candidates = index(msg);

        callback_candidates.for_each([&](auto index){
            callback_entries[index](msg, args...);
        });

        if (!callback_candidates) {
            CIB_ERROR("None of the registered callbacks claimed this message.");
        }
    }
};

} // namespace msg
