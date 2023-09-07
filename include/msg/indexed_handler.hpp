#pragma once

#include <log/log.hpp>
#include <msg/handler_interface.hpp>

namespace msg {

template <typename Field, typename Lookup> struct index {
    Lookup field_lookup;

    consteval index(Field, Lookup field_lookup_arg)
        : field_lookup{field_lookup_arg} {}

    constexpr auto operator()(auto const &data) const {
        return field_lookup[Field::extract(data)];
    }
};

template <typename... IndicesT> struct indices : IndicesT... {
    consteval explicit indices(IndicesT... index_args)
        : IndicesT{index_args}... {}

    constexpr auto operator()(auto const &data) const {
        return (this->IndicesT::operator()(data) & ...);
    }
};

template <typename BaseMsgT, typename... ExtraCallbackArgsT>
struct callback_args_t {};

template <typename BaseMsgT, typename... ExtraCallbackArgsT>
constexpr callback_args_t<BaseMsgT, ExtraCallbackArgsT...> callback_args{};

template <typename IndexT, typename CallbacksT, typename BaseMsgT,
          typename... ExtraCallbackArgsT>
struct indexed_handler : handler_interface<BaseMsgT, ExtraCallbackArgsT...> {
    using callback_func_t = void (*)(BaseMsgT const &,
                                     ExtraCallbackArgsT... args);

    IndexT index;
    CallbacksT callback_entries;

    constexpr explicit indexed_handler(
        callback_args_t<BaseMsgT, ExtraCallbackArgsT...>, IndexT new_index,
        CallbacksT new_callbacks)
        : index{new_index}, callback_entries{new_callbacks} {}

    auto is_match(BaseMsgT const &msg) const -> bool final {
        return !index(msg).empty();
    }

    void handle(BaseMsgT const &msg, ExtraCallbackArgsT... args) const final {
        auto const callback_candidates = index(msg);

        callback_candidates.for_each(
            [&](auto i) { callback_entries[i](msg, args...); });

        if (callback_candidates.empty()) {
            CIB_ERROR("None of the registered callbacks claimed this message.");
        }
    }
};

} // namespace msg
