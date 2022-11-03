#pragma once

#include <cib/tuple.hpp>
#include <log/log.hpp>
#include <sc/string_constant.hpp>

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
constexpr static event_handler<NameType, MatcherType, ActionType>
handle(NameType const &name, MatcherType const &matcher,
       ActionType const &action) {
    return {name, matcher, action};
}

template <typename ActionTypeT> struct default_event_handler {
    static constexpr bool is_default_handler = true;
    ActionTypeT action;
};

template <typename ActionType>
constexpr static default_event_handler<ActionType>
otherwise(ActionType const &action) {
    return {action};
}

template <typename NameType, typename EventType, typename... HandlerTypes>
constexpr static void process(NameType const &name, EventType const &event,
                              HandlerTypes const &...handlers) {
    const auto handlers_tuple = cib::make_tuple(handlers...);

    bool event_handled = false;

    handlers_tuple.for_each([&](auto handler) {
        if constexpr (!decltype(handler)::is_default_handler) {
            if (handler.matcher(event)) {
                INFO("{} - Processing [{}] due to match [{}]", name,
                     handler.name, handler.matcher.describe());
                handler.action();
                event_handled = true;
            }
        }
    });

    constexpr bool hasDefault =
        (HandlerTypes::is_default_handler || ... || false);

    if (!event_handled) {
        if constexpr (hasDefault) {
            handlers_tuple.for_each([&](auto handler) {
                if constexpr (decltype(handler)::is_default_handler) {
                    INFO("{} - Processing [default]", name);
                    handler.action();
                    event_handled = true;
                }
            });
        } else {
            const auto mismatch_descriptions =
                cib::transform(handlers_tuple, [&](auto handler) {
                    return format("    {} - F:({})\n"_sc, handler.name,
                                  handler.matcher.describe_match(event));
                });

            const auto mismatch_description = mismatch_descriptions.fold_left(
                [](auto lhs, auto rhs) { return lhs + rhs; });

            ERROR("{} - Received event that does not match any known "
                  "handler:\n{}",
                  name, mismatch_description);
        }
    }
}

template <bool value> struct always_t {
    template <typename EventType>
    [[nodiscard]] constexpr bool operator()(EventType const &) const {
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

template <typename... MatcherTypes> struct any_t {
    using MatchersType = cib::tuple<MatcherTypes...>;
    MatchersType matchers;

    constexpr any_t(MatchersType new_matchers) : matchers{new_matchers} {}

    template <typename EventType>
    [[nodiscard]] constexpr bool operator()(EventType const &event) const {
        return matchers.fold_left(false, [&](bool state, auto matcher) {
            return state || matcher(event);
        });
    }

    [[nodiscard]] constexpr auto describe() const {
        const auto matcher_descriptions = cib::transform(
            matchers, [&](auto m) { return "("_sc + m.describe() + ")"_sc; });

        return matcher_descriptions.fold_left(
            [](auto lhs, auto rhs) { return lhs + " || "_sc + rhs; });
    }

    template <typename EventType>
    [[nodiscard]] constexpr auto describe_match(EventType const &event) const {
        const auto matcher_descriptions = cib::transform(matchers, [&](auto m) {
            return format("{:c}:({})"_sc, m(event) ? 'T' : 'F',
                          m.describe_match(event));
        });

        return matcher_descriptions.fold_left(
            [](auto lhs, auto rhs) { return lhs + " || "_sc + rhs; });
    }
};

template <typename... MatcherTypes>
[[nodiscard]] constexpr auto any(MatcherTypes... matchers) {
    auto const matcher_tuple = cib::make_tuple(matchers...);

    auto const remaining_matcher_tuple =
        cib::filter(matcher_tuple, [](auto matcher) {
            using MatcherType = decltype(matcher);
            return sc::bool_<!std::is_same_v<MatcherType, always_t<false>>>;
        });

    auto const is_always_true = remaining_matcher_tuple.fold_right(
        sc::bool_<false>, [](auto matcher, auto already_found_always_true) {
            using MatcherType = decltype(matcher);
            auto constexpr matcher_is_always_true =
                std::is_same_v<MatcherType, always_t<true>>;
            return sc::bool_ < already_found_always_true ||
                   sc::bool_<matcher_is_always_true> > ;
        });

    if constexpr (is_always_true.value) {
        return always<true>;

    } else if constexpr (remaining_matcher_tuple.size() == 0) {
        // if there are no terms, then an OR-reduction should always be false
        return always<false>;

    } else if constexpr (remaining_matcher_tuple.size() == 1) {
        return remaining_matcher_tuple.get(cib::index_<0>);

    } else {
        return remaining_matcher_tuple.apply([&](auto... remaining_matchers) {
            return any_t<decltype(remaining_matchers)...>{
                remaining_matcher_tuple};
        });
    }
}

template <typename... MatcherTypes> struct all_t {
    using MatchersType = cib::tuple<MatcherTypes...>;
    MatchersType matchers;

    template <typename EventType>
    [[nodiscard]] constexpr bool operator()(EventType const &event) const {
        return matchers.fold_left(true, [&](bool state, auto matcher) {
            return state && matcher(event);
        });
    }

    [[nodiscard]] constexpr auto describe() const {
        const auto matcher_descriptions = cib::transform(
            matchers, [&](auto m) { return "("_sc + m.describe() + ")"_sc; });

        return matcher_descriptions.fold_left(
            [](auto lhs, auto rhs) { return lhs + " && "_sc + rhs; });
    }

    template <typename EventType>
    [[nodiscard]] constexpr auto describe_match(EventType const &event) const {
        const auto matcher_descriptions = cib::transform(matchers, [&](auto m) {
            return format("{:c}:({})"_sc, m(event) ? 'T' : 'F',
                          m.describe_match(event));
        });

        return matcher_descriptions.fold_left(
            [](auto lhs, auto rhs) { return lhs + " && "_sc + rhs; });
    }
};

template <typename... MatcherTypes>
[[nodiscard]] constexpr auto all(MatcherTypes... matchers) {
    auto const matcher_tuple = cib::make_tuple(matchers...);

    auto const remaining_matcher_tuple =
        cib::filter(matcher_tuple, [](auto matcher) {
            using MatcherType = decltype(matcher);
            return sc::bool_<!std::is_same_v<MatcherType, always_t<true>>>;
        });

    auto const is_always_false = remaining_matcher_tuple.fold_right(
        sc::bool_<false>, [](auto matcher, auto already_found_always_false) {
            using MatcherType = decltype(matcher);
            bool constexpr matcher_is_always_false =
                std::is_same_v<MatcherType, always_t<false>>;
            return sc::bool_ < already_found_always_false ||
                   sc::bool_<matcher_is_always_false> > ;
        });

    if constexpr (is_always_false.value) {
        return always<false>;

    } else if constexpr (remaining_matcher_tuple.size() == 0) {
        // if there are no terms, then an AND-reduction should always be true
        return always<true>;

    } else if constexpr (remaining_matcher_tuple.size() == 1) {
        return remaining_matcher_tuple.get(cib::index_<0>);

    } else {
        return remaining_matcher_tuple.apply([&](auto... remaining_matchers) {
            return all_t<decltype(remaining_matchers)...>{
                remaining_matcher_tuple};
        });
    }
}

template <typename MatcherType> struct not_t {
    MatcherType matcher;

    template <typename EventType>
    [[nodiscard]] constexpr bool operator()(EventType const &event) const {
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

    [[nodiscard]] constexpr bool operator()(EventType const &event) const {
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
    [[nodiscard]] constexpr bool operator()(Ts...) const {
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
