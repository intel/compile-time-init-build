#include <cib/cib.hpp>

#include <catch2/catch_test_macros.hpp>

using cib::detail::int_;

struct EmptyConfig {
    constexpr static auto config = cib::config<>();
};

TEST_CASE("an empty configuration should compile and initialize") {
    cib::nexus<EmptyConfig> nexus{};
    nexus.init();
}

template <int Id> static bool is_callback_invoked = false;

template <int Id, typename... Args>
struct TestCallback : public cib::callback_meta<Args...> {};

struct SimpleConfig {
    constexpr static auto config = cib::config(
        cib::exports<TestCallback<0>>,
        cib::extend<TestCallback<0>>([]() { is_callback_invoked<0> = true; }));
};

TEST_CASE("simple configuration with a single extension point and feature") {
    cib::nexus<SimpleConfig> nexus{};
    is_callback_invoked<0> = false;

    SECTION("services can be invoked directly from nexus") {
        nexus.service<TestCallback<0>>();
        REQUIRE(is_callback_invoked<0>);
    }

    SECTION(
        "nexus can be initialized services can be invoked from cib::service") {
        nexus.init();
        cib::service<TestCallback<0>>();
        REQUIRE(is_callback_invoked<0>);
    }
}

struct Foo {
    constexpr static auto config = cib::config(
        cib::exports<TestCallback<0>>,

        cib::extend<TestCallback<2>>([]() { is_callback_invoked<2> = true; }));
};

struct Bar {
    constexpr static auto config = cib::config(
        cib::extend<TestCallback<0>>([]() { is_callback_invoked<0> = true; }),
        cib::extend<TestCallback<1>>([]() { is_callback_invoked<1> = true; }));
};

struct Gorp {
    constexpr static auto config =
        cib::config(cib::exports<TestCallback<1>, TestCallback<2>>);
};

struct MediumConfig {
    constexpr static auto config = cib::config(cib::components<Foo, Bar, Gorp>);
};

TEST_CASE("configuration with multiple components, services, and features") {
    cib::nexus<MediumConfig> nexus{};
    is_callback_invoked<0> = false;
    is_callback_invoked<1> = false;
    is_callback_invoked<2> = false;

    SECTION("services can be invoked directly from nexus") {
        REQUIRE_FALSE(is_callback_invoked<0>);
        nexus.service<TestCallback<0>>();
        REQUIRE(is_callback_invoked<0>);

        REQUIRE_FALSE(is_callback_invoked<1>);
        nexus.service<TestCallback<1>>();
        REQUIRE(is_callback_invoked<1>);

        REQUIRE_FALSE(is_callback_invoked<2>);
        nexus.service<TestCallback<2>>();
        REQUIRE(is_callback_invoked<2>);
    }

    SECTION(
        "nexus can be initialized services can be invoked from cib::service") {
        nexus.init();

        REQUIRE_FALSE(is_callback_invoked<0>);
        cib::service<TestCallback<0>>();
        REQUIRE(is_callback_invoked<0>);

        REQUIRE_FALSE(is_callback_invoked<1>);
        cib::service<TestCallback<1>>();
        REQUIRE(is_callback_invoked<1>);

        REQUIRE_FALSE(is_callback_invoked<2>);
        cib::service<TestCallback<2>>();
        REQUIRE(is_callback_invoked<2>);
    }
}

TEST_CASE("test conditional expressions") {
    constexpr auto exp = cib::arg<int> == int_<42>;

    SECTION("true evaluation") {
        constexpr bool true_result = exp(int_<42>);
        REQUIRE(true_result);
    }

    SECTION("true evaluation with cib::apply") {
        constexpr bool true_result = cib::apply(exp, cib::make_tuple(int_<42>));
        REQUIRE(true_result);
    }

    SECTION("false evaluation") {
        constexpr bool false_result = cib::apply(exp, cib::make_tuple(int_<8>));
        REQUIRE_FALSE(false_result);
    }
}

struct SimpleConditionalComponent {
    constexpr static auto config = cib::config(cib::conditional(
        cib::arg<int> == int_<42>,
        cib::extend<TestCallback<0>>([]() { is_callback_invoked<0> = true; })));
};

template <int ConditionalValue> struct ConditionalTestProject {
    constexpr static auto config =
        cib::config(cib::args<ConditionalValue>, cib::exports<TestCallback<0>>,
                    cib::components<SimpleConditionalComponent>);
};

TEST_CASE("configuration with one conditional component") {
    is_callback_invoked<0> = false;

    SECTION("invoke with argument match") {
        cib::nexus<ConditionalTestProject<42>> nexus{};

        REQUIRE_FALSE(is_callback_invoked<0>);
        nexus.service<TestCallback<0>>();
        REQUIRE(is_callback_invoked<0>);
    }

    SECTION("invoke without argument match") {
        cib::nexus<ConditionalTestProject<7>> nexus{};

        REQUIRE_FALSE(is_callback_invoked<0>);
        nexus.service<TestCallback<0>>();
        REQUIRE_FALSE(is_callback_invoked<0>);
    }
}

template <int Id> struct ConditionalComponent {
    constexpr static auto config = cib::config(cib::conditional(
        cib::arg<int> == int_<Id>, cib::extend<TestCallback<Id>>([]() {
            is_callback_invoked<Id> = true;
        })));
};

template <int EnabledId> struct ConditionalConfig {
    constexpr static auto config = cib::config(
        cib::args<EnabledId>,

        cib::exports<TestCallback<0>, TestCallback<1>, TestCallback<2>>,

        cib::components<ConditionalComponent<0>, ConditionalComponent<1>,
                        ConditionalComponent<2>>);
};

TEST_CASE("configuration with conditional features") {
    is_callback_invoked<0> = false;
    is_callback_invoked<1> = false;
    is_callback_invoked<2> = false;

    SECTION("component 0 is enabled") {
        cib::nexus<ConditionalConfig<0>> nexus{};

        REQUIRE_FALSE(is_callback_invoked<0>);
        nexus.service<TestCallback<0>>();
        REQUIRE(is_callback_invoked<0>);

        REQUIRE_FALSE(is_callback_invoked<1>);
        nexus.service<TestCallback<1>>();
        REQUIRE_FALSE(is_callback_invoked<1>);

        REQUIRE_FALSE(is_callback_invoked<2>);
        nexus.service<TestCallback<2>>();
        REQUIRE_FALSE(is_callback_invoked<2>);
    }

    SECTION("component 1 is enabled") {
        cib::nexus<ConditionalConfig<1>> nexus{};

        REQUIRE_FALSE(is_callback_invoked<0>);
        nexus.service<TestCallback<0>>();
        REQUIRE_FALSE(is_callback_invoked<0>);

        REQUIRE_FALSE(is_callback_invoked<1>);
        nexus.service<TestCallback<1>>();
        REQUIRE(is_callback_invoked<1>);

        REQUIRE_FALSE(is_callback_invoked<2>);
        nexus.service<TestCallback<2>>();
        REQUIRE_FALSE(is_callback_invoked<2>);
    }

    SECTION("component 2 is enabled") {
        cib::nexus<ConditionalConfig<2>> nexus{};

        REQUIRE_FALSE(is_callback_invoked<0>);
        nexus.service<TestCallback<0>>();
        REQUIRE_FALSE(is_callback_invoked<0>);

        REQUIRE_FALSE(is_callback_invoked<1>);
        nexus.service<TestCallback<1>>();
        REQUIRE_FALSE(is_callback_invoked<1>);

        REQUIRE_FALSE(is_callback_invoked<2>);
        nexus.service<TestCallback<2>>();
        REQUIRE(is_callback_invoked<2>);
    }
}
