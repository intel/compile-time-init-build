#include "detail/compiler.hpp"
#include "detail/meta.hpp"
#include "builder_meta.hpp"

#include <tuple>
#include <type_traits>


#ifndef COMPILE_TIME_INIT_BUILD_CONFIG_HPP
#define COMPILE_TIME_INIT_BUILD_CONFIG_HPP


namespace cib {
    namespace detail {
        template<typename Lhs, typename Rhs>
        CIB_CONSTEXPR static auto is_same_v =
            std::is_same_v<
                std::remove_cv_t<std::remove_reference_t<Lhs>>,
                std::remove_cv_t<std::remove_reference_t<Rhs>>
            >;

        template<typename FirstT, typename... PathSegmentTs>
        struct path {
            using First = FirstT;
        };

        struct config_item {
            template<typename BuildersT, typename... Args>
            CIB_CONSTEVAL auto init(
                BuildersT const & buildersTuple,
                Args const & ...
            ) const {
                return buildersTuple;
            }

            template<typename... Args>
            CIB_CONSTEVAL auto exports_tuple(Args const & ...) const {
                return std::make_tuple();
            }
        };

        template<
            typename ExtensionPathT,
            typename... Args>
        struct extend_t : public config_item {
            std::tuple<Args...> argsTuple;

            CIB_CONSTEVAL extend_t(
                Args const & ... args
            )
                : argsTuple{args...}
            {
                // pass
            }

            template<
                typename TargetBuilderT,
                typename BuilderT>
            CIB_CONSTEVAL auto add(
                path<TargetBuilderT> const &,
                BuilderT const & b
            ) const {
                if constexpr (is_same_v<TargetBuilderT, decltype(b.first)>) {
                    return std::apply([&](auto const & ... args){
                        return std::pair(b.first, b.second.add(args...));
                    }, argsTuple);
                } else {
                    return b;
                }
            }

            template<
                typename TargetBuilderT,
                typename SubBuilderT,
                typename... SubBuilderTs,
                typename BuilderT>
            CIB_CONSTEVAL auto add(
                path<TargetBuilderT, SubBuilderT, SubBuilderTs...> const &,
                BuilderT const & b
            ) const {
                if constexpr (is_same_v<TargetBuilderT, decltype(b.first)>) {
                    return std::apply([&](auto const & ... args){
                        return std::pair(b.first, b.second.template add<SubBuilderT, SubBuilderTs...>(args...));
                    }, argsTuple);
                } else {
                    return b;
                }
            }

            template<typename BuildersT, typename... InitArgs>
            CIB_CONSTEVAL auto init(
                BuildersT const & buildersTuple,
                InitArgs const & ...
            ) const {
                return std::apply([&](auto const & ... builders){
                    static_assert(
                        (is_same_v<typename ExtensionPathT::First, decltype(builders.first)> + ... + 0) == 1,
                        "Extensions must match exactly one exported builder.");

                    return std::make_tuple(add(ExtensionPathT{}, builders)...);
                }, buildersTuple);
            }
        };

        template<typename... ExtensionPointTs>
        struct exports : public detail::config_item {
            template<typename... Args>
            CIB_CONSTEVAL auto exports_tuple(Args const & ...) const {
                return std::tuple<std::pair<ExtensionPointTs, traits::builder_t<ExtensionPointTs>>...>{};
            }
        };

        template<typename ComponentArgs, typename... ComponentTs>
        struct components : public detail::config_item {
            template<typename BuildersT, typename... Args>
            CIB_CONSTEVAL auto init(
                BuildersT const & buildersTuple,
                Args const & ... args
            ) const {
                return std::apply([&](auto const & ... componentArgs){
                    return detail::fold_right(std::tuple<ComponentTs...>{}, buildersTuple, [&](auto const & c, auto const & builders){
                        return c.config.init(builders, args..., componentArgs...);
                    });
                }, ComponentArgs::value);
            }

            template<typename... Args>
            CIB_CONSTEVAL auto exports_tuple(
                Args const & ... args
            ) const {
                return std::apply([&](auto const & ... componentArgs){
                    return std::tuple_cat(ComponentTs::config.exports_tuple(args..., componentArgs...)...);
                }, ComponentArgs::value);
            }
        };
    }

    template<auto Value>
    CIB_CONSTEXPR static auto as_constant_v = std::integral_constant<std::remove_cv_t<std::remove_reference_t<decltype(Value)>>, Value>{};

    template<auto... Args>
    struct args {
        static CIB_CONSTEXPR auto value = std::make_tuple(as_constant_v<Args>...);
    };

    template<typename Args, typename... Services>
    CIB_CONSTEXPR static detail::components<Args, Services...> components{};


    template<typename... ConfigTs>
    struct config : public detail::config_item {
        std::tuple<ConfigTs...> configsTuple;

        CIB_CONSTEVAL config(
            ConfigTs const & ... configs
        )
            : configsTuple{configs...}
        {
            // pass
        }

        template<typename BuildersT, typename... Args>
        CIB_CONSTEVAL auto init(
            BuildersT const & buildersTuple,
            Args const & ... args
        ) const {
            return detail::fold_right(configsTuple, buildersTuple, [&](auto const & c, auto builders){
                return c.init(builders, args...);
            });
        }

        template<typename... Args>
        CIB_CONSTEVAL auto exports_tuple(
            Args const & ... args
        ) const {
            return std::apply([&](auto const & ... configsPack){
                return std::tuple_cat(configsPack.exports_tuple(args...)...);
            }, configsTuple);
        }
    };

    template<typename... Services>
    CIB_CONSTEXPR static detail::exports<Services...> exports{};

    template<
        typename... ExtensionPointPathTs,
        typename... Args>
    [[nodiscard]] CIB_CONSTEVAL auto extend(Args const & ... args) {
        return detail::extend_t<detail::path<ExtensionPointPathTs...>, Args...>{args...};
    }

    template<
        typename ConditionT,
        typename... ConfigTs>
    struct conditional {
        CIB_CONSTEXPR static ConditionT condition{};
        config<ConfigTs...> body;

        CIB_CONSTEVAL conditional(
            ConditionT,
            ConfigTs const & ... configs
        )
           : body{configs...}
        {}

        template<
            typename BuildersT,
            typename... Args>
        CIB_CONSTEVAL auto init(
            BuildersT const & buildersTuple,
            Args...
        ) const {
            if constexpr (condition(Args{}...)) {
                return body.init(buildersTuple, Args{}...);
            } else {
                return buildersTuple;
            }
        }

        template<typename... Args>
        CIB_CONSTEVAL auto exports_tuple(Args...) const {
            if constexpr (condition(Args{}...)) {
                return body.exports_tuple(Args{}...);
            } else {
                return std::make_tuple();
            }
        }
    };



    template<
        typename Lhs,
        typename Rhs>
    struct equality {
        CIB_CONSTEXPR static Lhs lhs{};
        CIB_CONSTEXPR static Rhs rhs{};

        template<typename... Args>
        CIB_CONSTEVAL bool operator()(Args const & ... args) const {
            return lhs(args...) == rhs; // FIXME: this assumes the RHS is a literal value
        }
    };

    template<typename MatchType>
    struct arg_t {
        template<typename Rhs>
        CIB_CONSTEVAL equality<arg_t<MatchType>, Rhs> operator==(Rhs const &) const {
            return {};
        }

        template<typename... Args>
        CIB_CONSTEVAL auto operator()(Args... args) const {
            return detail::fold_right(std::make_tuple(args...), detail::int_<0>, [=](auto elem, auto state){
                using ElemType = typename std::remove_cv_t<std::remove_reference_t<decltype(elem)>>::value_type;

                if constexpr (std::is_same_v<ElemType, MatchType>) {
                    return elem;
                } else {
                    return state;
                }
            });
        }
    };

    template<typename ArgType>
    CIB_CONSTEXPR static arg_t<ArgType> arg{};
}


#endif //COMPILE_TIME_INIT_BUILD_CONFIG_HPP
