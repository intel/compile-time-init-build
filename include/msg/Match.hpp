#pragma once

#include <log/log.hpp>
#include <sc/string_constant.hpp>

#include <boost/hana.hpp>
#include <type_traits>

namespace match {
    namespace hana = boost::hana;

    template<
        typename NameTypeT,
        typename MatcherTypeT,
        typename ActionTypeT>
    struct EventHandler {
        static constexpr bool isDefaultHandler = false;
        NameTypeT name;
        MatcherTypeT matcher;
        ActionTypeT action;
    };

    template<
        typename NameType,
        typename MatcherType,
        typename ActionType>
    constexpr static EventHandler<NameType, MatcherType, ActionType> handle(
        NameType const & name,
        MatcherType const & matcher,
        ActionType const & action
    ) {
        return {name, matcher, action};
    }

    template<
        typename ActionTypeT>
    struct DefaultEventHandler {
        static constexpr bool isDefaultHandler = true;
        ActionTypeT action;
    };

    template<
        typename ActionType>
    constexpr static DefaultEventHandler<ActionType> otherwise(
        ActionType const & action
    ) {
        return {action};
    }

    template<typename NameType, typename EventType, typename... HandlerTypes>
    constexpr static void process(
        NameType const & name,
        EventType const & event,
        HandlerTypes const & ... handlers
    ) {
        const auto handlersTuple =
            hana::make_tuple(handlers...);

        bool eventHandled = false;

        hana::for_each(handlersTuple, [&](auto handler){
            if constexpr (!decltype(handler)::isDefaultHandler) {
                if (handler.matcher(event)) {
                    INFO("{} - Processing [{}] due to match [{}]",
                         name, handler.name, handler.matcher.describe());
                    handler.action();
                    eventHandled = true;
                }
            }
        });

        constexpr bool hasDefault =
            (HandlerTypes::isDefaultHandler || ... || false);

        if (!eventHandled) {
            if constexpr (hasDefault) {
                hana::for_each(handlersTuple, [&](auto handler){
                    if constexpr (decltype(handler)::isDefaultHandler) {
                        INFO("{} - Processing [default]", name);
                        handler.action();
                        eventHandled = true;
                    }
                });
            } else {
                const auto mismatchDescriptions =
                    hana::transform(handlersTuple, [&](auto handler){
                        return format("    {} - F:({})\n"_sc, handler.name, handler.matcher.describeMatch(event));
                    });

                const auto mismatchDescription =
                    hana::fold_left(mismatchDescriptions, [](auto lhs, auto rhs){
                        return lhs + rhs;
                    });

                ERROR(
                    "{} - Received event that does not match any known handler:\n{}",
                    name,
                    mismatchDescription);
            }
        }
    }



    template<bool value>
    struct Always {
        template<typename EventType>
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

        template<typename EventType>
        [[nodiscard]] constexpr auto describeMatch(EventType const &) const {
            return describe();
        }
    };

    template<bool value>
    constexpr Always<value> always{};

    template<typename... MatcherTypes>
    struct Any {
        using MatchersType = hana::tuple<MatcherTypes...>;
        MatchersType matchers;

        template<typename EventType>
        [[nodiscard]] constexpr bool operator()(EventType const & event) const {
            return hana::fold_left(matchers, false, [&](bool state, auto matcher){
                return state || matcher(event);
            });
        }

        [[nodiscard]] constexpr auto describe() const {
            const auto matcherDescriptions =
                hana::transform(matchers, [&](auto m){
                    return "("_sc + m.describe() + ")"_sc;
                });

            return hana::fold_left(matcherDescriptions, [](auto lhs, auto rhs){
                return lhs + " || "_sc + rhs;
            });
        }

        template<typename EventType>
        [[nodiscard]] constexpr auto describeMatch(EventType const & event) const {
            const auto matcherDescriptions =
                hana::transform(matchers, [&](auto m){
                    return format("{:c}:({})"_sc, m(event) ? 'T' : 'F', m.describeMatch(event));
                });

            return hana::fold_left(matcherDescriptions, [](auto lhs, auto rhs){
                return lhs + " || "_sc + rhs;
            });
        }
    };

    template<typename... MatcherTypes>
    [[nodiscard]] constexpr auto any(MatcherTypes... matchers) {
        auto const matcherTuple =
            hana::make_tuple(matchers...);

        auto const remainingMatcherTuple =
            hana::filter(matcherTuple, [](auto matcher){
                using MatcherType = decltype(matcher);
                return hana::bool_c<!std::is_same_v<MatcherType, Always<false>>>;
            });

        auto const isAlwaysTrue =
            hana::fold(remainingMatcherTuple, hana::bool_c<false>, [](auto alreadyFoundAlwaysTrue, auto matcher){
                using MatcherType = decltype(matcher);
                bool constexpr matcherIsAlwaysTrue = std::is_same_v<MatcherType, Always<true>>;
                return alreadyFoundAlwaysTrue || hana::bool_c<matcherIsAlwaysTrue>;
            });

        if constexpr (hana::value(isAlwaysTrue)) {
            return always<true>;

        } else if constexpr (hana::size(remainingMatcherTuple) == hana::size_c<0>) {
            // if there are no terms, then an OR-reduction should always be false
            return always<false>;

        } else if constexpr (hana::size(remainingMatcherTuple) == hana::size_c<1>) {
            return remainingMatcherTuple[sc::int_<0>];

        } else {
            return hana::unpack(remainingMatcherTuple, [&](auto... remainingMatchers){
                return Any<decltype(remainingMatchers)...>{remainingMatcherTuple};
            });
        }
    }

    template<typename... MatcherTypes>
    struct All {
        using MatchersType = hana::tuple<MatcherTypes...>;
        MatchersType matchers;

        template<typename EventType>
        [[nodiscard]] constexpr bool operator()(EventType const & event) const {
            return hana::fold_left(matchers, true, [&](bool state, auto matcher){
                return state && matcher(event);
            });
        }

        [[nodiscard]] constexpr auto describe() const {
            const auto matcherDescriptions =
                hana::transform(matchers, [&](auto m){
                    return "("_sc + m.describe() + ")"_sc;
                });

            return hana::fold_left(matcherDescriptions, [](auto lhs, auto rhs){
                return lhs + " && "_sc + rhs;
            });
        }

        template<typename EventType>
        [[nodiscard]] constexpr auto describeMatch(EventType const & event) const {
            const auto matcherDescriptions =
                hana::transform(matchers, [&](auto m){
                    return format("{:c}:({})"_sc, m(event) ? 'T' : 'F', m.describeMatch(event));
                });

            return hana::fold_left(matcherDescriptions, [](auto lhs, auto rhs){
                return lhs + " && "_sc + rhs;
            });
        }
    };

    template<typename... MatcherTypes>
    [[nodiscard]] constexpr auto all(MatcherTypes... matchers) {
        auto const matcherTuple =
            hana::make_tuple(matchers...);

        auto const remainingMatcherTuple =
            hana::filter(matcherTuple, [](auto matcher){
                using MatcherType = decltype(matcher);
                return hana::bool_c<!std::is_same_v<MatcherType, Always<true>>>;
            });

        auto const isAlwaysFalse =
            hana::fold(remainingMatcherTuple, hana::bool_c<false>, [](auto alreadyFoundAlwaysFalse, auto matcher){
                using MatcherType = decltype(matcher);
                bool constexpr matcherIsAlwaysFalse = std::is_same_v<MatcherType, Always<false>>;
                return alreadyFoundAlwaysFalse || hana::bool_c<matcherIsAlwaysFalse>;
            });

        if constexpr (hana::value(isAlwaysFalse)) {
            return always<false>;

        } else if constexpr (hana::size(remainingMatcherTuple) == hana::size_c<0>) {
            // if there are no terms, then an AND-reduction should always be true
            return always<true>;

        } else if constexpr (hana::size(remainingMatcherTuple) == hana::size_c<1>) {
            return remainingMatcherTuple[sc::int_<0>];

        } else {
            return hana::unpack(remainingMatcherTuple, [&](auto... remainingMatchers){
                return All<decltype(remainingMatchers)...>{remainingMatcherTuple};
            });
        }
    }

    template<typename MatcherType>
    struct Not {
        MatcherType matcher;

        template<typename EventType>
        [[nodiscard]] constexpr bool operator()(EventType const & event) const {
            return !matcher(event);
        }

        [[nodiscard]] constexpr auto describe() const {
            return format("!({})"_sc, matcher.describe());
        }

        template<typename EventType>
        [[nodiscard]] constexpr auto describeMatch(EventType const & event) const {
            return format("!{}"_sc, matcher.describeMatch(event));
        }
    };

    template<typename MatcherType>
    [[nodiscard]] constexpr auto not_(MatcherType matcher) {
        return Not<MatcherType>{matcher};
    }
}

template<typename EventType, typename DescType, typename PredType>
struct SimpleMatcher {
    static constexpr DescType description{};
    PredType predicate;

    [[nodiscard]] constexpr bool operator()(EventType const & event) const {
        return predicate(event);
    }

    [[nodiscard]] constexpr auto describe() const {
        return description;
    }

    [[nodiscard]] constexpr auto describeMatch(EventType const & event) const {
        return description;
    }

    [[nodiscard]] constexpr auto operator!() const {
        return match::not_(*this);
    }
};

template<typename DescType, typename PredType>
struct SimpleMatcher<void, DescType, PredType> {
    static constexpr DescType description{};
    PredType predicate;

    template<typename... Ts>
    [[nodiscard]] constexpr bool operator()(Ts...) const {
        return predicate();
    }

    [[nodiscard]] constexpr auto describe() const {
        return description;
    }

    template<typename... Ts>
    [[nodiscard]] constexpr auto describeMatch(Ts...) const {
        return description;
    }

    [[nodiscard]] constexpr auto operator!() const {
        return match::not_(*this);
    }
};

template<typename EventType, typename DescType, typename PredType>
[[nodiscard]] constexpr static auto matcher(
    DescType name,
    PredType predicate
) {
    return SimpleMatcher<EventType, DescType, PredType>{predicate};
}

