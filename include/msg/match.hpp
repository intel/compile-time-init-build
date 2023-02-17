#pragma once

#include <cib/tuple.hpp>
#include <log/log.hpp>
#include <sc/string_constant.hpp>

#include <functional>
#include <type_traits>

namespace match {
template <typename NameTypeT, typename MatcherTypeT, typename ActionTypeT>
struct event_handler {
    static constexpr bool is_default_handler = false;
    NameTypeT name;
    MatcherTypeT matcher;
    ActionTypeT action;
};

template <typename NameType, typename MatcherType, typename ActionType>
constexpr static auto handle(NameType const &name, MatcherType const &matcher,
                             ActionType const &action)
    -> event_handler<NameType, MatcherType, ActionType> {
    return {name, matcher, action};
}

template <typename ActionTypeT> struct default_event_handler {
    static constexpr bool is_default_handler = true;
    ActionTypeT action;
};

template <typename ActionType>
constexpr static auto otherwise(ActionType const &action)
    -> default_event_handler<ActionType> {
    return {action};
}

template <typename NameType, typename EventType, typename... HandlerTypes>
constexpr static void process(NameType const &name, EventType const &event,
                              HandlerTypes const &...handlers) {
    const auto handlers_tuple = cib::make_tuple(handlers...);

    bool event_handled = false;

    cib::for_each(
        [&](auto handler) {
            if constexpr (!decltype(handler)::is_default_handler) {
                if (handler.matcher(event)) {
                    CIB_INFO("{} - Processing [{}] due to match [{}]", name,
                             handler.name, handler.matcher.describe());
                    handler.action();
                    event_handled = true;
                }
            }
        },
        handlers_tuple);

    constexpr bool hasDefault = (HandlerTypes::is_default_handler or ...);

    if (!event_handled) {
        if constexpr (hasDefault) {
            cib::for_each(
                [&](auto handler) {
                    if constexpr (decltype(handler)::is_default_handler) {
                        CIB_INFO("{} - Processing [default]", name);
                        handler.action();
                        event_handled = true;
                    }
                },
                handlers_tuple);
        } else {
            const auto mismatch_descriptions = cib::transform(
                [&](auto handler) {
                    return format("    {} - F:({})\n"_sc, handler.name,
                                  handler.matcher.describe_match(event));
                },
                handlers_tuple);

            const auto mismatch_description = mismatch_descriptions.join(
                [](auto lhs, auto rhs) { return lhs + rhs; });

            CIB_ERROR("{} - Received event that does not match any known "
                      "handler:\n{}",
                      name, mismatch_description);
        }
    }
}

template <bool value> struct always_t {
    template <typename EventType>
    [[nodiscard]] constexpr auto operator()(EventType const &) const -> bool {
        return value;
    }

    [[nodiscard]] constexpr auto describe() const {
        if constexpr (value) {
            return "true"_sc;
        } else {
            return "false"_sc;
        }
    }

    template <typename EventType>
    [[nodiscard]] constexpr auto describe_match(EventType const &) const {
        return describe();
    }
};

template <bool value> constexpr always_t<value> always{};

namespace detail {
struct any_op : std::logical_or<> {
    static constexpr auto text = " || "_sc;
    static constexpr auto unit = false;
};
struct all_op : std::logical_and<> {
    static constexpr auto text = " && "_sc;
    static constexpr auto unit = true;
};

template <typename TOp, typename... MatcherTypes> struct logical_matcher {
    using MatchersType = cib::tuple<MatcherTypes...>;
    MatchersType matchers{};

    template <typename EventType>
    [[nodiscard]] constexpr auto operator()(EventType const &event) const
        -> bool {
        return matchers.fold_left(TOp::unit,
                                  [&](bool state, auto matcher) -> bool {
                                      return TOp{}(state, matcher(event));
                                  });
    }

    [[nodiscard]] constexpr auto describe() const {
        const auto matcher_descriptions = cib::transform(
            [](auto m) { return "("_sc + m.describe() + ")"_sc; }, matchers);
        return matcher_descriptions.join(
            [](auto lhs, auto rhs) { return lhs + TOp::text + rhs; });
    }

    template <typename EventType>
    [[nodiscard]] constexpr auto describe_match(EventType const &event) const {
        const auto matcher_descriptions = cib::transform(
            [&](auto m) {
                return format("{:c}:({})"_sc, m(event) ? 'T' : 'F',
                              m.describe_match(event));
            },
            matchers);
        return matcher_descriptions.join(
            [](auto lhs, auto rhs) { return lhs + TOp::text + rhs; });
    }
};

template <typename TOp> struct match_op {
    template <typename T>
    using fn = std::bool_constant<not std::is_same_v<always_t<TOp::unit>, T>>;
};

template <typename TOp, typename... MatcherTypes>
[[nodiscard]] constexpr auto
make_logical_matcher(MatcherTypes const &...matchers) {
    if constexpr ((std::is_same_v<MatcherTypes, always_t<not TOp::unit>> or
                   ...)) {
        return always<not TOp::unit>;
    } else {
        auto const remaining_matcher_tuple =
            cib::filter<match_op<TOp>::template fn>(
                cib::make_tuple(matchers...));
        if constexpr (remaining_matcher_tuple.size() == 0) {
            return always<TOp::unit>;
        } else if constexpr (remaining_matcher_tuple.size() == 1) {
            return remaining_matcher_tuple[cib::index<0>];
        } else {
            return remaining_matcher_tuple.apply(
                [&](auto... remaining_matchers) {
                    return logical_matcher<TOp,
                                           decltype(remaining_matchers)...>{
                        remaining_matcher_tuple};
                });
        }
    }
}
} // namespace detail

template <typename... MatcherTypes>
using any_t = detail::logical_matcher<detail::any_op, MatcherTypes...>;
template <typename... MatcherTypes>
using all_t = detail::logical_matcher<detail::all_op, MatcherTypes...>;

template <typename... MatcherTypes>
[[nodiscard]] constexpr auto any(MatcherTypes const &...matchers) {
    return detail::make_logical_matcher<detail::any_op>(matchers...);
}

template <typename... MatcherTypes>
[[nodiscard]] constexpr auto all(MatcherTypes const &...matchers) {
    return detail::make_logical_matcher<detail::all_op>(matchers...);
}

template <typename MatcherType> struct not_t {
    MatcherType matcher;

    template <typename EventType>
    [[nodiscard]] constexpr auto operator()(EventType const &event) const
        -> bool {
        return !matcher(event);
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("!({})"_sc, matcher.describe());
    }

    template <typename EventType>
    [[nodiscard]] constexpr auto describe_match(EventType const &event) const {
        return format("!{}"_sc, matcher.describe_match(event));
    }
};

template <typename MatcherType>
[[nodiscard]] constexpr auto not_(MatcherType matcher) {
    return not_t<MatcherType>{matcher};
}
} // namespace match

template <typename EventType, typename DescType, typename PredType>
struct simple_matcher_t {
    static constexpr DescType description{};
    PredType predicate;

    [[nodiscard]] constexpr auto operator()(EventType const &event) const
        -> bool {
        return predicate(event);
    }

    [[nodiscard]] constexpr auto describe() const { return description; }

    [[nodiscard]] constexpr auto describe_match(EventType const &) const {
        return description;
    }

    [[nodiscard]] constexpr auto operator!() const {
        return match::not_(*this);
    }
};

template <typename DescType, typename PredType>
struct simple_matcher_t<void, DescType, PredType> {
    static constexpr DescType description{};
    PredType predicate;

    template <typename... Ts>
    [[nodiscard]] constexpr auto operator()(Ts...) const -> bool {
        return predicate();
    }

    [[nodiscard]] constexpr auto describe() const { return description; }

    template <typename... Ts>
    [[nodiscard]] constexpr auto describe_match(Ts...) const {
        return description;
    }

    [[nodiscard]] constexpr auto operator!() const {
        return match::not_(*this);
    }
};

template <typename EventType, typename DescType, typename PredType>
[[nodiscard]] constexpr static auto matcher(DescType, PredType predicate) {
    return simple_matcher_t<EventType, DescType, PredType>{predicate};
}
