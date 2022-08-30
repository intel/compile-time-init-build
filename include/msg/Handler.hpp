#pragma once

#include <msg/Message.hpp>
#include <type_traits>
#include <tuple>

#include <container/Vector.hpp>
#include <container/Array.hpp>
#include <cib/tuple.hpp>

#include <log/log.hpp>

namespace msg {
    template<
        typename BaseMsgT,
        typename... ExtraCallbackArgsT>
    struct Callback {
        [[nodiscard]] virtual bool isMatch(BaseMsgT const & msg) const = 0;
        [[nodiscard]] virtual bool handle(BaseMsgT const & msg, ExtraCallbackArgsT const & ... args) const = 0;
        virtual void logMismatch(BaseMsgT const & msg) const = 0;
    };

    template<typename T>
    using remove_cvref_t = typename std::remove_cv< typename std::remove_reference<T>::type >::type;



    template<typename...>
    struct CallbackTypes {};

    template<typename...>
    struct ExtraCallbackArgs {};

    template<
        typename BaseMsgT,
        typename ExtraCallbackArgsListT,
        typename NameTypeT,
        typename MatchMsgTypeT,
        typename CallbackTypeListT>
    struct CallbackImpl;

    template<typename CallableT>
    struct func_args {
         using msg_type = typename func_args<decltype(&remove_cvref_t<CallableT>::operator())>::msg_type;
         using type = typename func_args<decltype(&remove_cvref_t<CallableT>::operator())>::type;
    };

    template<typename DataIterableT, typename... ArgTs>
    struct func_args<void(DataIterableT, ArgTs...)> {
        using msg_type = remove_cvref_t<DataIterableT>;
        using type = cib::tuple<ArgTs...>;
    };

    template<typename DataIterableT, typename... ArgTs>
    struct func_args<void(DataIterableT, ArgTs...) const> {
        using msg_type = remove_cvref_t<DataIterableT>;
        using type = cib::tuple<ArgTs...>;
    };

    template<typename DataIterableT, typename T, typename... ArgTs>
    struct func_args<void(T::*)(DataIterableT, ArgTs...)> {
        using msg_type = remove_cvref_t<DataIterableT>;
        using type = cib::tuple<ArgTs...>;
    };

    template<typename DataIterableT, typename T, typename... ArgTs>
    struct func_args<void(T::*)(DataIterableT, ArgTs...) const> {
        using msg_type = remove_cvref_t<DataIterableT>;
        using type = cib::tuple<ArgTs...>;
    };

    template<typename CallableT>
    using func_args_t = typename func_args<CallableT>::type;

    template<typename CallableT>
    constexpr func_args_t<CallableT> func_args_v{};


    template<typename CallbackType>
    using msg_type_t = typename func_args<CallbackType>::msg_type;


    template <
        typename CallableT,
        typename DataIterableT,
        typename... ExtraCallbackArgsT>
    void dispatchSingleCallable(
        CallableT const & callable,
        DataIterableT const & data,
        ExtraCallbackArgsT const & ... args
    ) {
        auto const providedArgsTuple = std::tuple(args...);
        auto const requiredArgsTuple =
            cib::transform(
                func_args_v<CallableT>,
                [&](auto requiredArg){
                    using RequiredArgType = decltype(requiredArg);
                    return std::get<RequiredArgType>(providedArgsTuple);
                }
            );

        requiredArgsTuple.apply([&](auto const & ... requiredArgs){
            using MsgType = msg_type_t<decltype(callable)>;
            MsgType const msg{data};

            if (msg.isValid()) {
                callable(msg, requiredArgs...);
            }
        });
    }

    /**
     * A Class that defines a message callback and provides methods for validating
     * and handling incoming messages.
     */
    template<
        typename BaseMsgT,
        typename... ExtraCallbackArgsT,
        typename NameTypeT,
        typename MatchMsgTypeT,
        typename... CallableTypesT>
    class CallbackImpl<
        BaseMsgT,
        ExtraCallbackArgs<ExtraCallbackArgsT...>,
        NameTypeT,
        MatchMsgTypeT,
        CallbackTypes<CallableTypesT...>
    >
        : public Callback<BaseMsgT, ExtraCallbackArgsT...>
    {
    private:
        constexpr static NameTypeT name{};

        MatchMsgTypeT matchMsg;
        cib::tuple<CallableTypesT...> callbacks;

        template<typename DataIterableType>
        void dispatch(DataIterableType const & data, ExtraCallbackArgsT const & ... args) const {
            callbacks.for_each([&](auto callback){
                dispatchSingleCallable(callback, data, args...);
            });
        }

        [[nodiscard]] constexpr auto matchAnyCallback() const {
            auto const matchers = cib::transform(callbacks, [&](auto callback){
                using MsgType = msg_type_t<decltype(callback)>;
                return isValidMsg<MsgType>(match::always<true>);
            });

            return matchers.apply([](auto... matchersPack){
                return match::any(matchersPack...);
            });
        }

    public:
        constexpr CallbackImpl(
            MatchMsgTypeT const & matchMsg,
            CallableTypesT const & ... callbackArgs
        )
            : matchMsg(matchMsg)
            , callbacks(cib::make_tuple(callbackArgs...))
        {
            callbacks.for_each([](auto callback) {
                static_assert(
                    !std::is_same<decltype(callback), std::nullptr_t>::value,
                    "Function pointer specified for callback can't be null");
            });
        }

        [[nodiscard]] bool isMatch(BaseMsgT const & msg) const final override {
            return match::all(matchMsg, matchAnyCallback())(msg);
        }

        [[nodiscard]] bool handle(BaseMsgT const & msg, ExtraCallbackArgsT const & ... args) const final override {
            auto matchHandler =
                match::all(matchMsg, matchAnyCallback());

            if (matchHandler(msg)) {
                INFO(
                    "Incoming message matched [{}], because [{}], executing callback",
                    name, matchHandler.describe());

                dispatch(msg.data, args...);

                return true;
            }

            return false;
        }

        void logMismatch(BaseMsgT const & msg) const final override {
            INFO("    {} - F:({})", name,
                 match::all(matchMsg, matchAnyCallback()).describeMatch(msg));
        }
    };




    template <
        typename BaseMsgT,
        size_t NumMsgCallbacksT,
        typename... ExtraCallbackArgsT>
    class Handler {
    private:
        using CallbackType = Callback<BaseMsgT, ExtraCallbackArgsT...>;
        using CallbacksType = Array<CallbackType const *, NumMsgCallbacksT>;
        CallbacksType callbacks{};

    public:
        constexpr Handler(
            CallbacksType callbacksArg
        )
            : callbacks{callbacksArg}
        {
            // pass
        }

        constexpr Handler(Handler const & rhs) = default;
        constexpr Handler &operator=(Handler const & rhs) = default;

        bool isMatch(BaseMsgT const & msg) const {
            for (auto callback : callbacks) {
                if (
                    callback->isMatch(msg)
                ) {
                    return true;
                }
            }

            return false;
        }

        void handle(BaseMsgT const & msg, ExtraCallbackArgsT const & ... args) const {
            bool foundValidCallback = false;
            for (auto callback : callbacks) {
                if (callback->handle(msg, args...)) {
                    foundValidCallback = true;
                }
            }

            if (!foundValidCallback) {
                ERROR("None of the registered callbacks claimed this message:");

                for (auto callback : callbacks) {
                    callback->logMismatch(msg);
                }
            }
        }
    };


    template<
        typename AbstractInterface,
        typename Derived,
        typename BaseMsgT,
        typename... ExtraCallbackArgsT>
    class HandlerBuilder {
    public:
        static constexpr auto MAX_SIZE = 256;
        using CallbacksType = Vector<Callback<BaseMsgT, ExtraCallbackArgsT...> const *, MAX_SIZE>;

    private:
        CallbacksType callbacks;

        template<size_t NumMsgCallbacksT>
        [[nodiscard]] constexpr Handler<BaseMsgT, NumMsgCallbacksT, ExtraCallbackArgsT...> buildBackend() const {
            Array<Callback<BaseMsgT, ExtraCallbackArgsT...> const *, NumMsgCallbacksT> newMsgCallbacks;

            for (std::size_t i = 0; i < callbacks.size(); i++) {
                newMsgCallbacks[i] = callbacks[i];
            }

            return {newMsgCallbacks};
        }

    public:
        constexpr HandlerBuilder()
            : callbacks{}
        {
            // pass
        }

        constexpr void add(Callback<BaseMsgT, ExtraCallbackArgsT...> const & callback) {
            callbacks.push(&callback);
        }

        [[nodiscard]] constexpr size_t getNumCallbacks() const {
            return callbacks.size();
        }

        template<size_t NumMsgCallbacksT>
        [[nodiscard]] constexpr auto internalBuild() const {
            auto const backend = buildBackend<NumMsgCallbacksT>();
            auto const frontend = Derived::buildFrontend(backend);

            return frontend;
        }


        ///////////////////////////////////////////////////////////////////////////
        ///
        /// Everything below is for the cib extension interface. It lets cib know
        /// this builder supports the cib pattern and how to build it.
        ///
        ///////////////////////////////////////////////////////////////////////////
        /**
         * Never called, but the return type is used by cib to determine what the
         * abstract interface is.
         */
        AbstractInterface * base() const;

        template<typename BuilderValue>
        static constexpr auto build() {
            auto constexpr handlerBuilder = BuilderValue::value;
            auto constexpr config = handlerBuilder.getNumCallbacks();
            return handlerBuilder.template internalBuild<config>();
        }
    };

    template<
        typename BaseMsgT,
        typename... ExtraCallbackArgsT>
    auto callback =
        [](auto name, auto matchMsg, auto... callbacks) ->
            CallbackImpl<
                BaseMsgT,
                ExtraCallbackArgs<ExtraCallbackArgsT...>,
                decltype(name),
                decltype(matchMsg),
                CallbackTypes<decltype(callbacks)...>
            >
        {
            return {matchMsg, callbacks...};
        };
}