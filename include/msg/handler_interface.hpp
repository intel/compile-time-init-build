#pragma once

#include <stdx/ct_conversions.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/panic.hpp>
#include <stdx/type_traits.hpp>

namespace msg {
template <typename MsgBase, typename... ExtraCallbackArgs>
struct handler_interface {
    virtual auto is_match(MsgBase const &msg) const -> bool = 0;

    virtual auto handle(MsgBase const &msg,
                        ExtraCallbackArgs... extra_args) const -> bool = 0;
};

namespace detail {
template <typename M> consteval auto name_for_msg() {
    if constexpr (stdx::named<M>) {
        return stdx::name_of_v<M>;
    } else {
        constexpr auto name = stdx::type_as_string<M>();
        return stdx::ct_string<name.size() + 1>{name};
    }
}
} // namespace detail

template <typename MsgBase, typename... ExtraCallbackArgs>
struct uninitialized_handler_t
    : handler_interface<MsgBase, ExtraCallbackArgs...> {
    auto is_match(MsgBase const &) const -> bool override { return false; }

    auto handle(MsgBase const &, ExtraCallbackArgs...) const -> bool override {
        using namespace stdx::literals;
        stdx::panic<"Attempting to handle msg ("_cts +
                    detail::name_for_msg<MsgBase>() +
                    ") before service is initialized"_cts>();
        return false;
    }
};
} // namespace msg
