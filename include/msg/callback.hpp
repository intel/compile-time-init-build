#pragma once

#include <log/log.hpp>
#include <match/ops.hpp>
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
          stdx::callable F, typename... ExtraArgs>
struct callback {
    [[nodiscard]] auto is_match(auto const &data) const -> bool {
        auto view = typename Msg::view_t{data};
        return matcher(view);
    }

    [[nodiscard]] auto handle(auto const &data, ExtraArgs const &...args) const
        -> bool {
        auto view = typename Msg::view_t{data};
        if (matcher(view)) {
            CIB_INFO("Incoming message matched [{}], because [{}], executing "
                     "callback",
                     stdx::ct_string_to_type<Name, sc::string_constant>(),
                     matcher.describe());
            callable(view, args...);
            return true;
        }
        return false;
    }

    auto log_mismatch(auto const &data) const -> void {
        auto view = typename Msg::view_t{data};

        CIB_INFO("    {} - F:({})",
                 stdx::ct_string_to_type<Name, sc::string_constant>(),
                 matcher.describe_match(view));
    }

    using msg_t = Msg;
    using matcher_t = M;
    using callable_t = F;

    template <match::matcher NewM>
    using rebind_matcher = callback<Name, Msg, NewM, F, ExtraArgs...>;

    constexpr static auto name = Name;
    [[no_unique_address]] matcher_t matcher;
    [[no_unique_address]] callable_t callable;
};
} // namespace detail

template <stdx::ct_string Name, typename Msg, typename... ExtraArgs>
constexpr auto callback = []<match::matcher M, stdx::callable F>(M, F &&f) {
    using matcher_t =
        decltype(match::sum_of_products(M{} and typename Msg::matcher_t{}));
    return detail::callback<Name, Msg, matcher_t, std::remove_cvref_t<F>,
                            ExtraArgs...>{matcher_t{}, std::forward<F>(f)};
};
} // namespace msg
