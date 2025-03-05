#pragma once

#include <log/log.hpp>
#include <msg/handler_interface.hpp>
#include <msg/message.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ranges.hpp>

#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

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

template <typename Index, typename Callbacks, typename MsgBase,
          typename... ExtraCallbackArgs>
struct indexed_handler : handler_interface<MsgBase, ExtraCallbackArgs...> {
    Index index;
    Callbacks callback_entries;

    template <typename Idx, typename CBs>
    constexpr explicit indexed_handler(Idx &&idx, CBs &&cbs)
        : index{std::forward<Idx>(idx)},
          callback_entries{std::forward<CBs>(cbs)} {}

    // This function may lie... it may claim to match when it doesn't because
    // there are further conditions on the non-indexed parts of the matcher
    auto is_match(MsgBase const &msg) const -> bool final {
        return not index(msg).none();
    }

    __attribute__((flatten)) auto handle(MsgBase const &msg,
                                         ExtraCallbackArgs... args) const
        -> bool final {
        auto const callback_candidates = index(msg);

        bool const handled = transform_reduce(
            [&](auto i) -> bool { return callback_entries[i](msg, args...); },
            std::logical_or{}, false, callback_candidates);

        if (not handled) {
            CIB_ERROR(
                "None of the registered callbacks ({}) claimed this message.",
                sc::uint_<stdx::tuple_size_v<Callbacks>>);
        }
        return handled;
    }
};

template <typename MsgBase, typename... ExtraCallbackArgs>
constexpr auto make_indexed_handler = []<typename Idx, typename CBs>(
                                          Idx &&idx, CBs &&cbs) {
    return indexed_handler<std::remove_cvref_t<Idx>, std::remove_cvref_t<CBs>,
                           MsgBase, ExtraCallbackArgs...>{
        std::forward<Idx>(idx), std::forward<CBs>(cbs)};
};
} // namespace msg
