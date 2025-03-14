#pragma once

#include <async/completion_tags.hpp>
#include <async/concepts.hpp>
#include <async/connect.hpp>
#include <async/debug_context.hpp>
#include <async/incite_on.hpp>
#include <async/just.hpp>
#include <async/schedulers/trigger_scheduler.hpp>
#include <async/start.hpp>
#include <async/then.hpp>

#include <stdx/concepts.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/type_traits.hpp>

#include <type_traits>
#include <utility>

namespace msg {
namespace _send_recv {
template <typename S>
concept valid_send_action =
    requires { typename std::remove_cvref_t<S>::is_send_action; };

template <typename F> struct send_action {
    [[no_unique_address]] F f;
    using is_send_action = void;
};
template <typename F> send_action(F) -> send_action<F>;

template <stdx::ct_string Name, typename... Args> struct pipeable {
    template <typename Adaptor> struct type {
        [[no_unique_address]] Adaptor a;

      private:
        template <valid_send_action S, stdx::same_as_unqualified<type> Self>
        friend constexpr auto operator|(S &&s, Self &&self) -> async::sender
            auto {
            return async::just(std::forward<S>(s).f) |
                   async::incite_on(async::trigger_scheduler<Name, Args...>{}) |
                   std::forward<Self>(self).a;
        }
    };

    template <typename T> type(T) -> type<T>;
};
} // namespace _send_recv

template <typename F, typename... Args>
constexpr auto send(F &&f, Args &&...args) {
    return _send_recv::send_action{
        [f = std::forward<F>(f), ... as = std::forward<Args>(args)] {
            return std::move(f)(std::move(as)...);
        }};
}

template <stdx::ct_string Name, typename... RecvArgs, typename F,
          typename... Args>
[[nodiscard]] constexpr auto then_receive(F &&f, Args &&...args) {
    return typename _send_recv::pipeable<Name, RecvArgs...>::type{async::then(
        [f = std::forward<F>(f), ... args = std::forward<Args>(args)](
            RecvArgs const &...as) { return f(as..., args...); })};
}

template <stdx::ct_string Name, _send_recv::valid_send_action S, typename F,
          typename... Args>
[[nodiscard]] constexpr auto then_receive(S &&s, F &&f, Args &&...args)
    -> async::sender auto {
    return std::forward<S>(s) |
           then_receive<Name>(std::forward<F>(f), std::forward<Args>(args)...);
}
} // namespace msg
