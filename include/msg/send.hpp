#pragma once

#include <async/completion_tags.hpp>
#include <async/concepts.hpp>
#include <async/connect.hpp>
#include <async/schedulers/trigger_scheduler.hpp>
#include <async/start.hpp>
#include <async/then.hpp>

#include <stdx/concepts.hpp>
#include <stdx/ct_string.hpp>

#include <type_traits>
#include <utility>

namespace msg {
namespace _send_recv {
template <typename Sched>
using scheduler_sender = decltype(std::declval<Sched>().schedule());

template <typename S>
concept valid_send_action =
    requires { typename std::remove_cvref_t<S>::is_send_action; };

template <valid_send_action SA, typename Sched, typename Rcvr>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
struct op_state {
    template <stdx::same_as_unqualified<SA> S, typename Sch, typename R>
    constexpr op_state(S &&s, Sch &&sch, R &&r)
        : send(s), ops{async::connect(std::forward<Sch>(sch).schedule(),
                                      std::forward<R>(r))} {}
    constexpr op_state(op_state &&) = delete;

    using ops_t = async::connect_result_t<scheduler_sender<Sched>, Rcvr>;

    constexpr auto start() & -> void {
        async::start(ops);
        send();
    }

    SA send;
    ops_t ops;
};

template <valid_send_action SA, typename Sched> struct sender {
    using is_sender = void;

    [[no_unique_address]] SA send;
    [[no_unique_address]] Sched sched;

  public:
    template <async::receiver R>
    [[nodiscard]] constexpr auto
    connect(R &&r) && -> op_state<SA, Sched, std::remove_cvref_t<R>> {
        async::check_connect<sender &&, R>();
        return {std::move(send), std::move(sched), std::forward<R>(r)};
    }

    template <async::receiver R>
    [[nodiscard]] constexpr auto
    connect(R &&r) const & -> op_state<SA, Sched, std::remove_cvref_t<R>> {
        async::check_connect<sender, R>();
        return {send, sched, std::forward<R>(r)};
    }

    template <typename Env>
    [[nodiscard]] constexpr static auto get_completion_signatures(Env const &)
        -> async::completion_signatures_of_t<scheduler_sender<Sched>, Env> {
        return {};
    }
};

template <typename Sndr, typename Sched>
sender(Sndr, Sched) -> sender<Sndr, Sched>;

template <typename F> struct send_action : F {
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
            return _send_recv::sender{
                       std::forward<S>(s),
                       async::trigger_scheduler<Name, Args...>{}} |
                   std::forward<Self>(self).a;
        }
    };

    template <typename T> type(T) -> type<T>;
};
} // namespace _send_recv

template <typename F, typename... Args>
constexpr auto send(F &&f, Args &&...args) {
    return _send_recv::send_action{
        [f = std::forward<F>(f), ... args = std::forward<Args>(args)]() {
            return f(args...);
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
[[nodiscard]] constexpr auto then_receive(S &&s, F &&f,
                                          Args &&...args) -> async::sender
    auto {
    return std::forward<S>(s) |
           then_receive<Name>(std::forward<F>(f), std::forward<Args>(args)...);
}
} // namespace msg
