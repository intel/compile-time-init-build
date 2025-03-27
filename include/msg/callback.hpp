#pragma once

#include <log/log.hpp>
#include <match/ops.hpp>
#include <match/predicate.hpp>
#include <msg/message.hpp>

#include <stdx/concepts.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <type_traits>
#include <utility>

namespace msg {
namespace detail {

/**
 * A Class that defines a message callback and provides methods for validating
 * and handling incoming messages.
 */
template <stdx::ct_string Name, typename Msg, match::matcher M,
          stdx::callable F>
struct callback {
    [[nodiscard]] auto is_match(auto const &data) const -> bool {
        return msg::call_with_message<Msg>(matcher, data);
    }

    template <stdx::ct_string Extra = "", typename... Args>
    [[nodiscard]] auto handle(auto const &data, Args &&...args) const -> bool {
        CIB_LOG_ENV(logging::get_level, logging::level::INFO);
        if (msg::call_with_message<Msg>(matcher, data)) {
            CIB_APPEND_LOG_ENV(typename Msg::env_t);
            CIB_LOG("Incoming message matched [{}], because [{}]{}, executing "
                    "callback",
                    stdx::cts_t<Name>{}, matcher.describe(),
                    stdx::cts_t<Extra>{});
            msg::call_with_message<Msg>(callable, data,
                                        std::forward<Args>(args)...);
            return true;
        }
        return false;
    }

    auto log_mismatch(auto const &data) const -> void {
        CIB_LOG_ENV(logging::get_level, logging::level::INFO);
        {
            CIB_APPEND_LOG_ENV(typename Msg::env_t);
            CIB_LOG(
                "    {} - F:({})", stdx::cts_t<Name>{},
                msg::call_with_message<Msg>(
                    [&]<typename T>(T &&t) -> decltype(matcher.describe_match(
                                               std::forward<T>(t))) {
                        return matcher.describe_match(std::forward<T>(t));
                    },
                    data));
        }
    }

    using msg_t = Msg;
    using matcher_t = M;
    using callable_t = F;

    template <match::matcher NewM>
    using rebind_matcher = callback<Name, Msg, NewM, F>;

    constexpr static auto name = Name;
    [[no_unique_address]] matcher_t matcher;
    [[no_unique_address]] callable_t callable;
};

template <stdx::ct_string Name, typename Msg> struct callback_construct_t {
    template <match::matcher M, stdx::callable F>
    [[nodiscard]] constexpr auto operator()(M, F &&f) const {
        using ::operator and;
        using matcher_t =
            decltype(match::sum_of_products(M{} and typename Msg::matcher_t{}));
        return callback<Name, Msg, matcher_t, std::remove_cvref_t<F>>{
            matcher_t{}, std::forward<F>(f)};
    }

    template <msg::matcher_maker M, stdx::callable F>
    [[nodiscard]] constexpr auto operator()(M, F &&f) const {
        return this->operator()(M::template make_matcher<Msg>(),
                                std::forward<F>(f));
    }

  private:
    template <typename N> struct matching_name {
        template <typename Field>
        using fn = std::is_same<N, typename Field::name_t>;
    };
};

template <typename Cond, stdx::ct_string Name, typename Msg, match::matcher M,
          stdx::callable F>
constexpr auto make_runtime_conditional(Cond, callback<Name, Msg, M, F> cb) {
    using ::operator and;

    auto predicate = match::predicate<Cond::ct_name>(
        [](auto...) -> bool { return static_cast<bool>(Cond{}); });
    using predicate_t = decltype(predicate);

    auto new_matcher = match::sum_of_products(M{} and predicate_t{});
    using new_matcher_t = decltype(new_matcher);

    using new_cb_t = callback<Name, Msg, new_matcher_t, F>;

    return new_cb_t{new_matcher, cb.callable};
}

} // namespace detail

template <stdx::ct_string Name, typename Msg>
constexpr inline auto callback = detail::callback_construct_t<Name, Msg>{};
} // namespace msg
