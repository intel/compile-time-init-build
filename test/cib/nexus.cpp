#include <cib/cib.hpp>

#include <catch2/catch_test_macros.hpp>

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

template <auto V> struct SimpleConditionalComponent {
    constexpr static auto when_v_is_42 =
        cib::constexpr_condition<"when_v_is_42">([]() { return V == 42; });

    constexpr static auto config = cib::config(when_v_is_42(
        cib::extend<TestCallback<0>>([]() { is_callback_invoked<0> = true; })));
};

template <int ConditionalValue> struct ConditionalTestProject {
    constexpr static auto config = cib::config(
        cib::exports<TestCallback<0>>,
        cib::components<SimpleConditionalComponent<ConditionalValue>>);
};

TEST_CASE("configuration with one constexpr conditional component") {
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

template <int EnId, int Id> struct ConditionalComponent {
    constexpr static auto when_id_matches =
        cib::constexpr_condition<"when_id_matches">(
            []() { return EnId == Id; });

    constexpr static auto config =
        cib::config(when_id_matches(cib::extend<TestCallback<Id>>(
            []() { is_callback_invoked<Id> = true; })));
};

template <int EnabledId> struct ConditionalConfig {
    constexpr static auto config = cib::config(
        cib::exports<TestCallback<0>, TestCallback<1>, TestCallback<2>>,

        cib::components<ConditionalComponent<EnabledId, 0>,
                        ConditionalComponent<EnabledId, 1>,
                        ConditionalComponent<EnabledId, 2>>);
};

TEST_CASE("configuration with constexpr conditional features") {
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
