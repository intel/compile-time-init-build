#pragma once


#include <utility>
#include <type_traits>
#include <tuple>
#include <array>


///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// compiler/language support
///
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __cpp_constinit
    #if defined(__clang__)
        #define CIB_CONSTINIT [[clang::require_constant_initialization]]
    #else
        #define CIB_CONSTINIT
    #endif
#else
    #define CIB_CONSTINIT constinit
#endif

#ifndef __cpp_consteval
    #define CIB_CONSTEVAL constexpr
#else
    #define CIB_CONSTEVAL consteval
#endif

#define CIB_CONSTEXPR constexpr


///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// meta programming
///
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace cib::detail {
    template<int value>
    CIB_CONSTEXPR auto int_ = std::integral_constant<int, value>{};

    template<
        typename ValueT,
        typename CallableT>
    struct fold_helper {
        ValueT const & value;
        CallableT const & op;

        CIB_CONSTEVAL fold_helper(
            ValueT const & value,
            CallableT const & op
        )
            : value{value}
            , op{op}
        {}

        template<typename StateT>
        [[nodiscard]] CIB_CONSTEVAL inline auto operator+(
            StateT const & state
        ) const {
            return op(value, state);
        }
    };

    template<
        typename... T,
        typename InitT,
        typename CallableT>
    [[nodiscard]] CIB_CONSTEVAL inline static auto fold_right(
        std::tuple<T...> const & t,
        InitT const & init,
        CallableT const & op
    ) {
        return std::apply([&](auto const & ... e){
            return (fold_helper{e, op} + ... + init);
        }, t);
    }

    template<
        typename T,
        T... i,
        typename CallableT>
    CIB_CONSTEVAL inline static void for_each(
        std::integer_sequence<T, i...> const &,
        CallableT const & op
    ) {
        (op(std::integral_constant<int, i>{}) , ...);
    }

    template<
        typename T,
        T size,
        typename CallableT>
    CIB_CONSTEVAL inline void for_each(
        std::integral_constant<T, size> const &,
        CallableT const & op
    ) {
        CIB_CONSTEXPR auto seq = std::make_integer_sequence<T, size>{};
        for_each(seq, op);
    }

    template<
        typename... Ts,
        typename CallableT>
    CIB_CONSTEVAL inline void for_each(
        std::tuple<Ts...> const & tuple,
        CallableT const & op
    ) {
        std::apply([&](auto const & ... pack){
            (op(pack) , ...);
        }, tuple);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// builder_meta
///
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace cib {
    template<
        typename BuilderT,
        typename InterfaceT>
    struct builder_meta {
        BuilderT builder();
        InterfaceT interface();
    };

    namespace traits {
        template<typename MetaT>
        struct builder {
            using type = decltype(std::declval<MetaT>().builder());
        };

        template<typename MetaT>
        using builder_t = typename builder<MetaT>::type;

        template<typename MetaT>
        CIB_CONSTEXPR builder_t<MetaT> builder_v = {};

        template<typename MetaT>
        struct interface {
            using type = decltype(std::declval<MetaT>().interface());
        };

        template<typename MetaT>
        using interface_t = typename interface<MetaT>::type;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// config
///
///////////////////////////////////////////////////////////////////////////////////////////////////
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
            typename... ArgTs>
        struct extend_t : public config_item {
            std::tuple<ArgTs...> argsTuple;

            CIB_CONSTEVAL extend_t(
                ArgTs const & ... args
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

            template<typename BuildersT, typename... Args>
            CIB_CONSTEVAL auto init(
                BuildersT const & buildersTuple,
                Args const & ...
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
        struct exports : public config_item {
            template<typename... Args>
            CIB_CONSTEVAL auto exports_tuple(Args const & ...) const {
                return std::tuple<std::pair<ExtensionPointTs, traits::builder_t<ExtensionPointTs>>...>{};
            }
        };

        template<typename ComponentArgs, typename... ComponentTs>
        struct components_t : public detail::config_item {
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

    template<auto... Args>
    struct args {
        static CIB_CONSTEXPR auto value = std::make_tuple(Args...);
    };

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

    template<typename... ExtensionPointTs>
    CIB_CONSTEXPR auto exports = detail::exports<ExtensionPointTs...>{};

    template<
        typename... ExtensionPointPathTs,
        typename... ArgTs>
    [[nodiscard]] CIB_CONSTEVAL auto extend(ArgTs const & ... args) {
        return detail::extend_t<detail::path<ExtensionPointPathTs...>, ArgTs...>{args...};
    }

    template<typename... ComponentTs>
    CIB_CONSTEXPR auto components = detail::components_t<ComponentTs...>{};

    template<
        typename ConditionT,
        typename... ConfigTs>
    struct conditional {
        ConditionT condition;
        config<ConfigTs...> body;

        CIB_CONSTEVAL conditional(
            ConditionT const & condition,
            ConfigTs const & ... configs
        )
           : body{configs...}
           , condition{condition}
        {}

        template<
            typename BuildersT,
            typename... Args>
        CIB_CONSTEVAL auto init(
            BuildersT const & buildersTuple,
            Args const & ... args
        ) const {
            if (condition(args...)) {
                return body.init(buildersTuple, args...);
            } else {
                return buildersTuple;
            }
        }

        template<typename... Args>
        CIB_CONSTEVAL auto exports_tuple(Args const & ... args) const {
            if (condition(args...)) {
                return body.exports_tuple(args...);
            } else {
                return std::make_tuple();
            }
        }
    };

    template<
        typename Lhs,
        typename Rhs>
    struct equality {
        Lhs lhs;
        Rhs rhs;

        CIB_CONSTEVAL equality(Lhs const & lhs, Rhs const & rhs) : lhs{lhs}, rhs{rhs} {}

        template<typename... Args>
        CIB_CONSTEVAL bool operator()(Args const & ... args) const {
            return lhs(args...) == rhs; // FIXME: this assumes the RHS is a literal value
        }
    };

    template<typename ArgType>
    struct arg_t {
        template<typename Rhs>
        CIB_CONSTEVAL auto operator==(Rhs const & rhs) const {
            return equality{*this, rhs};
        }

        template<typename... Args>
        CIB_CONSTEVAL auto operator()(Args const &... args) const {
            return std::get<ArgType>(std::make_tuple(args...));
        }
    };

    template<typename ArgType>
    CIB_CONSTEXPR static arg_t<ArgType> arg{};
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// built
///
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace cib {
    /**
     * Pointer to a concrete built handler.
     *
     * @tparam Tag Type name of the Builder.
     */
    template<typename Tag>
    traits::interface_t<Tag> built;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// nexus
///
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace cib {
    template<typename ConfigT>
    struct initialized_builders {
        CIB_CONSTEXPR static auto value = ConfigT::config.init(ConfigT::config.exports_tuple());
        using type = decltype(value);
    };

    template<typename ConfigT>
    using initialized_builders_t = typename initialized_builders<ConfigT>::type;

    template<typename ConfigT>
    CIB_CONSTEXPR static auto & initialized_builders_v = initialized_builders<ConfigT>::value;

    template<typename ConfigT, typename Tag>
    struct initialized {
        CIB_CONSTEXPR static auto value = detail::fold_right(initialized_builders_v<ConfigT>, 0, [](auto b, auto retval){
            if constexpr (std::is_same_v<decltype(b.first), Tag>) {
                return b.second;
            } else {
                return retval;
            }
        });
    };

    template<typename ConfigT, typename T>
    using builder_t = decltype(initialized<ConfigT, T>::value.template build<initialized<ConfigT, T>>());

    template<typename ConfigT, typename T>
    CIB_CONSTINIT static inline builder_t<ConfigT, T> builder = initialized<ConfigT, T>::value.template build<initialized<ConfigT, T>>();

    /**
     * Type trait for building a Builder and storing its Implementation.
     *
     * @tparam Builders
     *      A type with a 'static constexpr value' field that contains a std::tuple of all the initialized builders.
     *
     * @tparam Tag
     *      The typename of a the Builder to be built into an implementation.
     */
    /**
     * Build the builder. Passing in a type with a 'static constexpr value' member field is a pattern that works
     * for all builder/implementation combinations. This 'value' field is where the built Builder is stored.
     */
    template<typename ConfigT>
    static void init() {
        detail::for_each(initialized_builders_v<ConfigT>, [](auto b){
            // Tag/CleanTag is the type name of the builder_meta in the tuple
            using Tag = decltype(b.first);
            using CleanTag = std::remove_cv_t<std::remove_reference_t<Tag>>;

            // the built implementation is stored in Build<>::value
            auto & builtValue = builder<ConfigT, CleanTag>;
            using BuiltType = std::remove_reference_t<decltype(builtValue)>;

            // if the built type is a pointer, then it is a function pointer and we should return its value
            // directly to the 'built<>' abstract interface variable.
            if constexpr(std::is_pointer_v<BuiltType>) {
                built<CleanTag> = builtValue;

            // if the built type is not a pointer, then it is a class and the 'built<>' variable is a pointer to
            // the base class. we will need to get a pointer to the builtValue in order to initialize 'built<>'.
            } else {
                built<CleanTag> = &builtValue;
            }
        });
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// callback
///
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace cib {
    /**
     * Extension point/builder for simple callbacks.
     *
     * Modules can add their own callback function to this builder to be executed when
     * the builder is executed with the same function arguments.
     *
     * @tparam SizeT
     *      Maximum number of callbacks that may be registered.
     *
     * @tparam ArgTs
     *      List of argument types that must be passed into the callback when it is invoked.
     */
    template<int SizeT = 0, typename... ArgTs>
    struct callback {
    private:
        using func_ptr_t = void(*)(ArgTs...);

        std::array<func_ptr_t, SizeT> funcs;

        template<typename BuilderValue>
        static void run(ArgTs... args) {
            CIB_CONSTEXPR auto handlerBuilder = BuilderValue::value;
            CIB_CONSTEXPR auto numFuncs = std::integral_constant<int, SizeT>{};

            detail::for_each(numFuncs, [&](auto i){
                CIB_CONSTEXPR auto func = handlerBuilder.funcs[i];
                func(args...);
            });
        }

    public:
        CIB_CONSTEVAL callback() = default;

        template<typename PrevFuncsT>
        CIB_CONSTEVAL callback(
            PrevFuncsT const & prev_funcs,
            func_ptr_t new_func
        )
            : funcs{}
        {
            for (int i = 0; i < prev_funcs.size(); i++) {
                funcs[i] = prev_funcs[i];
            }

            funcs[SizeT - 1] = new_func;
        }

        // cib uses "add(...)" to add features to service builders
        CIB_CONSTEVAL auto add(func_ptr_t const & func) const {
            return callback<SizeT + 1, ArgTs...>{funcs, func};
        }

        /**
         * Build and return a function pointer to the implemented callback builder. Used
         * by cib library to automatically build an initialized builder. Do not call.
         *
         * @tparam BuilderValue
         *      Struct that contains a "static constexpr auto value" field with the initialized
         *      builder.
         *
         * @return
         *      Function pointer to callback builder.
         */
        template<typename BuilderValue>
        [[nodiscard]] CIB_CONSTEVAL static auto build() {
            return run<BuilderValue>;
        }
    };

    template<typename... ArgTs>
    struct callback_meta :
        public cib::builder_meta<
            callback<0, ArgTs...>,
            void(*)(ArgTs...)>
    {};
}








