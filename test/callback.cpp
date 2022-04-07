#include <cib/cib.hpp>

#include <type_traits>
#include <tuple>

#include <catch2/catch_test_macros.hpp>

template<typename BuilderValue>
constexpr static auto build() {
    return BuilderValue::value.template build<BuilderValue>();
}

template<
    typename BuilderMeta,
    typename BuiltCallback>
constexpr static bool built_is_convertable_to_interface(BuiltCallback) {
    using interface_type = cib::traits::interface_t<BuilderMeta>;
    return std::is_convertible_v<BuiltCallback, interface_type>;
}

struct EmptyCallbackNoArgs {
    using meta = cib::callback_meta<>;
    constexpr static auto value = cib::traits::builder_t<meta>{};
};

TEST_CASE("empty callback with no args", "[callback]") {
    constexpr auto built_callback =
        build<EmptyCallbackNoArgs>();

    SECTION("can be called without issue") {
        built_callback();
    }

    SECTION("built type is convertable to the interface type") {
        REQUIRE(built_is_convertable_to_interface<EmptyCallbackNoArgs::meta>(built_callback));
    }
}

template<int Id>
static bool is_callback_invoked = false;

struct CallbackNoArgsWithSingleExtension {
    using meta = cib::callback_meta<>;
    constexpr static auto value = [](){
        auto const builder = cib::traits::builder_t<meta>{};
        return builder.add([](){
            is_callback_invoked<0> = true;
        });
    }();
};

TEST_CASE("callback with no args with single extension", "[callback]") {
    constexpr auto built_callback =
        build<CallbackNoArgsWithSingleExtension>();

    is_callback_invoked<0> = false;

    SECTION("can be called and callback will be invoked") {
        built_callback();
        REQUIRE(is_callback_invoked<0>);
    }

    SECTION("built type is convertable to the interface type") {
        REQUIRE(built_is_convertable_to_interface<CallbackNoArgsWithSingleExtension::meta>(built_callback));
    }
}

struct CallbackNoArgsWithMultipleExtensions {
    using meta = cib::callback_meta<>;

    static void extension_one() {
        is_callback_invoked<1> = true;
    }

    static constexpr auto extension_two = [](){
        is_callback_invoked<2> = true;
    };

    constexpr static auto value = [](){
        auto const builder = cib::traits::builder_t<meta>{};

        return builder.add([](){
            is_callback_invoked<0> = true;
        }).add(
            extension_one
        ).add(
            extension_two
        );
    }();
};

TEST_CASE("callback with no args with multiple extensions", "[callback]") {
    constexpr auto built_callback =
        build<CallbackNoArgsWithMultipleExtensions>();

    is_callback_invoked<0> = false;
    is_callback_invoked<1> = false;
    is_callback_invoked<2> = false;

    SECTION("can be called and callback will be invoked") {
        built_callback();
        REQUIRE(is_callback_invoked<0>);
        REQUIRE(is_callback_invoked<1>);
        REQUIRE(is_callback_invoked<2>);
    }

    SECTION("built type is convertable to the interface type") {
        REQUIRE(built_is_convertable_to_interface<CallbackNoArgsWithMultipleExtensions::meta>(built_callback));
    }
}


struct CallbackWithArgsWithNoExtensions {
    using meta = cib::callback_meta<int, bool>;
    constexpr static auto value = cib::traits::builder_t<meta>{};
};

TEST_CASE("callback with args with no extensions", "[callback]") {
    constexpr auto built_callback =
        build<CallbackWithArgsWithNoExtensions>();

    SECTION("can be called") {
        built_callback(42, true);
    }

    SECTION("built type is convertable to the interface type") {
        REQUIRE(built_is_convertable_to_interface<CallbackWithArgsWithNoExtensions::meta>(built_callback));
    }
}



template<int Id, typename... ArgTypes>
static std::tuple<ArgTypes...> callback_args{};

struct CallbackWithArgsWithMultipleExtensions {
    using meta = cib::callback_meta<int, bool>;

    static void extension_one(int a, bool b) {
        is_callback_invoked<1> = true;
        callback_args<1, int, bool> = {a, b};
    }

    static constexpr auto extension_two = [](int a, bool b){
        is_callback_invoked<2> = true;
        callback_args<2, int, bool> = {a, b};
    };

    constexpr static auto value = [](){
        auto const builder = cib::traits::builder_t<meta>{};

        return builder.add([](int a, bool b){
            is_callback_invoked<0> = true;
            callback_args<0, int, bool> = {a, b};

        }).add(
            extension_one
        ).add(
            extension_two
        );
    }();
};

TEST_CASE("callback with args with multiple extensions", "[callback]") {
    constexpr auto built_callback =
        build<CallbackWithArgsWithMultipleExtensions>();

    SECTION("can be called") {
        built_callback(42, true);

        REQUIRE(std::get<0>(callback_args<0, int, bool>) == 42);
        REQUIRE(std::get<1>(callback_args<0, int, bool>) == true);

        REQUIRE(std::get<0>(callback_args<1, int, bool>) == 42);
        REQUIRE(std::get<1>(callback_args<1, int, bool>) == true);

        REQUIRE(std::get<0>(callback_args<2, int, bool>) == 42);
        REQUIRE(std::get<1>(callback_args<2, int, bool>) == true);


        built_callback(12, false);

        REQUIRE(std::get<0>(callback_args<0, int, bool>) == 12);
        REQUIRE(std::get<1>(callback_args<0, int, bool>) == false);

        REQUIRE(std::get<0>(callback_args<1, int, bool>) == 12);
        REQUIRE(std::get<1>(callback_args<1, int, bool>) == false);

        REQUIRE(std::get<0>(callback_args<2, int, bool>) == 12);
        REQUIRE(std::get<1>(callback_args<2, int, bool>) == false);
    }

    SECTION("built type is convertable to the interface type") {
        REQUIRE(built_is_convertable_to_interface<CallbackWithArgsWithMultipleExtensions::meta>(built_callback));
    }
}