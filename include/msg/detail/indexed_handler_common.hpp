#pragma once

#include <log/log.hpp>
#include <msg/handler_interface.hpp>

#include <stdx/compiler.hpp>
#include <stdx/concepts.hpp>

namespace msg {

template <typename Field, typename Lookup> struct index {
    Lookup field_lookup;

    CONSTEVAL index(Field, Lookup field_lookup_arg)
        : field_lookup{field_lookup_arg} {}

    template <typename Msg> constexpr auto operator()(Msg const &msg) const {
        if constexpr (stdx::range<Msg>) {
            return field_lookup[Field::extract(msg)];
        } else {
            return field_lookup[Field::extract(std::data(msg))];
        }
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
        return not index(msg).none();
    }

    auto handle(BaseMsgT const &msg, ExtraCallbackArgsT... args) const
        -> bool final {
        auto const callback_candidates = index(msg);

        for_each([&](auto i) { callback_entries[i](msg, args...); },
                 callback_candidates);

        bool const candidates_found = !callback_candidates.none();

        if (not candidates_found) {
            CIB_ERROR("None of the registered callbacks claimed this message.");
        }

        return candidates_found;
    }
};

} // namespace msg
