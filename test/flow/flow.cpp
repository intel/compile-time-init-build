#include <catch2/catch_test_macros.hpp>

#include <flow/flow.hpp>
#include <cib/cib.hpp>

#include <string>

namespace {
    auto actual = std::string("");


    constexpr auto milestone0 = flow::milestone("milestone0"_sc);
    constexpr auto milestone1 = flow::milestone("milestone1"_sc);


    constexpr static auto a = flow::action("a"_sc, [] {
        actual += "a";
    });

    constexpr auto b = flow::action("b"_sc, [] {
        actual += "b";
    });

    constexpr auto c = flow::action("c"_sc, [] {
        actual += "c";
    });

    constexpr auto d = flow::action("d"_sc, [] {
        actual += "d";
    });


    TEST_CASE("build and run empty flow", "[flow]") {
        flow::builder<> builder;
        auto const flow = builder.internal_build<0>();
        flow();
    }

    TEST_CASE("add single action", "[flow]") {
        flow::builder<> builder;
        actual = "";

        builder.add(a);

        auto const flow = builder.internal_build<1>();
        flow();

        REQUIRE(flow.getBuildStatus() == flow::build_status::SUCCESS);
        REQUIRE(actual == "a");
    }

    TEST_CASE("two milestone linear before dependency", "[flow]") {
        flow::builder<> builder;
        actual = "";

        /*
         * A fundamental feature of flows are their ability to traverse a graph
         * of dependencies and execute a series of tasks in the correct order.
         * The Flow::add function is used to add these ordering relationships to
         * the flow.
         */
        builder.add(a >> milestone0);

        /*
         * Because we previously created a dependent relationship between
         * 'action' and 'done', the flow knows to execute 'action' before it is
         * finished.
         */
        auto const flow = builder.internal_build<2>();
        flow();

        REQUIRE(actual == "a");
    }

    TEST_CASE("actions get executed once", "[flow]") {
        flow::builder<> builder;
        actual = "";

        builder.add(a >> milestone0);
        builder.add(a >> milestone1);
        builder.add(milestone0 >> milestone1);

        auto const flow = builder.internal_build<3>();
        flow();

        REQUIRE(actual == "a");
    }

    TEST_CASE("two milestone linear after dependency", "[flow]") {
        flow::builder<> builder;
        actual = "";

        builder.add(a >> milestone0);

        builder.add(milestone0 >> milestone1);

        builder.add(milestone0 >> b >> milestone1);

        auto const flow = builder.internal_build<4>();
        flow();

        REQUIRE(actual == "ab");
    }

    TEST_CASE("three milestone linear before and after dependency", "[flow]") {
        flow::builder<> builder;
        actual = "";


        builder.add(a >> b >> c);

        auto const flow = builder.internal_build<3>();
        flow();

        REQUIRE(actual == "abc");
    }

    TEST_CASE("just two actions in order", "[flow]") {
        flow::builder<> builder;
        actual = "";

        builder.add(a >> b);

        auto const flow = builder.internal_build<2>();
        flow();

        REQUIRE(actual == "ab");
    }

    TEST_CASE("insert action between two actions", "[flow]") {
        flow::builder<> builder;
        actual = "";

        builder.add(a >> c);

        builder.add(a >> b >> c);

        auto const flow = builder.internal_build<3>();
        flow();

        REQUIRE(actual == "abc");
    }

    TEST_CASE("add single parallel 2", "[flow]") {
        flow::builder<> builder;
        actual = "";

        builder.add(a && b);

        auto const flow = builder.internal_build<2>();
        flow();

        REQUIRE(actual.find('a') != std::string::npos);
        REQUIRE(actual.find('b') != std::string::npos);
        REQUIRE(actual.size() == 2);
    }

    TEST_CASE("add single parallel 3", "[flow]") {
        flow::builder<> builder;
        actual = "";

        builder.add(a && b && c);

        auto const flow = builder.internal_build<3>();
        flow();

        REQUIRE(actual.find('a') != std::string::npos);
        REQUIRE(actual.find('b') != std::string::npos);
        REQUIRE(actual.find('c') != std::string::npos);
        REQUIRE(actual.size() == 3);
    }

    TEST_CASE("add single parallel 3 with later dependency 1", "[flow]") {
        flow::builder<> builder;
        actual = "";

        builder.add(a && b && c);
        builder.add(c >> a);

        auto const flow = builder.internal_build<3>();
        flow();

        REQUIRE(actual.find('a') != std::string::npos);
        REQUIRE(actual.find('b') != std::string::npos);
        REQUIRE(actual.find('c') != std::string::npos);
        REQUIRE(actual.find('c') < actual.find('a'));
        REQUIRE(actual.size() == 3);
    }

    TEST_CASE("add single parallel 3 with later dependency 2", "[flow]") {
        flow::builder<> builder;
        actual = "";

        builder.add(a && b && c);
        builder.add(a >> c);

        auto const flow = builder.internal_build<3>();
        flow();

        REQUIRE(actual.find('a') != std::string::npos);
        REQUIRE(actual.find('b') != std::string::npos);
        REQUIRE(actual.find('c') != std::string::npos);
        REQUIRE(actual.find('a') < actual.find('c'));
        REQUIRE(actual.size() == 3);
    }

    TEST_CASE("add parallel rhs", "[flow]") {
        flow::builder<> builder;
        actual = "";

        builder.add(a >> (b && c));

        auto const flow = builder.internal_build<3>();
        flow();

        REQUIRE(actual.find('a') != std::string::npos);
        REQUIRE(actual.find('b') != std::string::npos);
        REQUIRE(actual.find('c') != std::string::npos);
        REQUIRE(actual.find('a') < actual.find('b'));
        REQUIRE(actual.find('a') < actual.find('c'));
        REQUIRE(actual.size() == 3);
    }

    TEST_CASE("add parallel lhs", "[flow]") {
        flow::builder<> builder;
        actual = "";

        builder.add((a && b) >> c);

        auto const flow = builder.internal_build<3>();
        flow();

        REQUIRE(actual.find('a') != std::string::npos);
        REQUIRE(actual.find('b') != std::string::npos);
        REQUIRE(actual.find('c') != std::string::npos);
        REQUIRE(actual.find('a') < actual.find('c'));
        REQUIRE(actual.find('b') < actual.find('c'));
        REQUIRE(actual.size() == 3);
    }

    TEST_CASE("add parallel in the middle", "[flow]") {
        flow::builder<decltype("MiddleParallelFlow"_sc)> builder;
        actual = "";

        builder.add(a >> (b && c) >> d);

        auto const flow = builder.internal_build<4>();
        flow();

        REQUIRE(actual.find('a') != std::string::npos);
        REQUIRE(actual.find('b') != std::string::npos);
        REQUIRE(actual.find('c') != std::string::npos);
        REQUIRE(actual.find('d') != std::string::npos);

        REQUIRE(actual.find('a') < actual.find('b'));
        REQUIRE(actual.find('a') < actual.find('c'));

        REQUIRE(actual.find('b') < actual.find('d'));
        REQUIRE(actual.find('c') < actual.find('d'));

        REQUIRE(actual.size() == 4);
    }

    TEST_CASE("add dependency lhs", "[flow]") {
        flow::builder<> builder;
        actual = "";

        builder.add((a >> b) && c);

        auto const flow = builder.internal_build<3>();
        flow();

        REQUIRE(actual.find('a') != std::string::npos);
        REQUIRE(actual.find('b') != std::string::npos);
        REQUIRE(actual.find('c') != std::string::npos);

        REQUIRE(actual.find('a') < actual.find('b'));

        REQUIRE(actual.size() == 3);
    }

    TEST_CASE("add dependency rhs", "[flow]") {
        flow::builder<> builder;
        actual = "";

        builder.add(a && (b >> c));

        auto const flow = builder.internal_build<3>();
        flow();

        REQUIRE(actual.find('a') != std::string::npos);
        REQUIRE(actual.find('b') != std::string::npos);
        REQUIRE(actual.find('c') != std::string::npos);

        REQUIRE(actual.find('b') < actual.find('c'));

        REQUIRE(actual.size() == 3);
    }

    struct TestFlowAlpha : public flow::service<> {};
    struct TestFlowBeta : public flow::service<> {};

    struct SingleFlowSingleActionConfig {
        constexpr static auto config =
            cib::config(
                cib::exports<TestFlowAlpha>,
                cib::extend<TestFlowAlpha>(a)
        );
    };

    TEST_CASE("add single action through cib::nexus", "[flow]") {
        cib::nexus<SingleFlowSingleActionConfig> nexus{};
        actual = "";

        nexus.service<TestFlowAlpha>();

        REQUIRE(actual == "a");
    }

    struct MultiFlowMultiActionConfig {
        constexpr static auto config =
            cib::config(
                cib::exports<
                    TestFlowAlpha,
                    TestFlowBeta>,

                cib::extend<TestFlowAlpha>(a),
                cib::extend<TestFlowAlpha>(a >> b),

                cib::extend<TestFlowBeta>(d),
                cib::extend<TestFlowBeta>(c >> d)
        );
    };

    TEST_CASE("add multi action through cib::nexus", "[flow]") {
        cib::nexus<MultiFlowMultiActionConfig> nexus{};
        actual = "";

        nexus.service<TestFlowAlpha>();
        nexus.service<TestFlowBeta>();

        REQUIRE(actual == "abcd");
    }

    TEST_CASE("add multi action through cib::nexus, run through cib::service", "[flow]") {
        cib::nexus<MultiFlowMultiActionConfig> nexus{};
        nexus.init();

        actual = "";

        cib::service<TestFlowAlpha>();
        cib::service<TestFlowBeta>();

        REQUIRE(actual == "abcd");
    }
}
