#include <catch2/catch_test_macros.hpp>

#include <flow/flow.hpp>

#include <string>


auto actual = std::string("");


auto milestone0 = flow::milestone("milestone0"_sc);
auto milestone1 = flow::milestone("milestone1"_sc);


constexpr auto a = flow::action("a"_sc, [] {
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
    flow::Builder<> builder;
    auto const flow = builder.internalBuild<0>();
    flow();
}

TEST_CASE("add single action", "[flow]") {
    flow::Builder<> builder;
    actual = "";

    builder.add(a);

    auto const flow = builder.internalBuild<1>();
    flow();

    REQUIRE(flow.getBuildStatus() == flow::build_status::SUCCESS);
    REQUIRE(actual == "a");
}

TEST_CASE("two milestone linear before dependency", "[flow]") {
    flow::Builder<> builder;
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
    auto const flow = builder.internalBuild<2>();
    flow();

    REQUIRE(actual == "a");
}

TEST_CASE("actions get executed once", "[flow]") {
    flow::Builder<> builder;
    actual = "";

    builder.add(a >> milestone0);
    builder.add(a >> milestone1);
    builder.add(milestone0 >> milestone1);

    auto const flow = builder.internalBuild<3>();
    flow();

    REQUIRE(actual == "a");
}

TEST_CASE("two milestone linear after dependency", "[flow]") {
    flow::Builder<> builder;
    actual = "";

    auto milestone0 = flow::milestone("milestone0"_sc);
    auto milestone1 = flow::milestone("milestone1"_sc);

    builder.add(a >> milestone0);

    builder.add(milestone0 >> milestone1);

    builder.add(milestone0 >> b >> milestone1);

    auto const flow = builder.internalBuild<4>();
    flow();

    REQUIRE(actual == "ab");
}

TEST_CASE("three milestone linear before and after dependency", "[flow]") {
    flow::Builder<> builder;
    actual = "";


    builder.add(a >> b >> c);

    auto const flow = builder.internalBuild<3>();
    flow();

    REQUIRE(actual == "abc");
}

TEST_CASE("just two actions in order", "[flow]") {
    flow::Builder<> builder;
    actual = "";

    builder.add(a >> b);

    auto const flow = builder.internalBuild<2>();
    flow();

    REQUIRE(actual == "ab");
}

TEST_CASE("insert action between two actions", "[flow]") {
    flow::Builder<> builder;
    actual = "";

    builder.add(a >> c);

    builder.add(a >> b >> c);

    auto const flow = builder.internalBuild<3>();
    flow();

    REQUIRE(actual == "abc");
}

TEST_CASE("add single parallel 2", "[flow]") {
    flow::Builder<> builder;
    actual = "";

    builder.add(a && b);

    auto const flow = builder.internalBuild<2>();
    flow();

    REQUIRE(actual.find('a') != std::string::npos);
    REQUIRE(actual.find('b') != std::string::npos);
    REQUIRE(actual.size() == 2);
}

TEST_CASE("add single parallel 3", "[flow]") {
    flow::Builder<> builder;
    actual = "";

    builder.add(a && b && c);

    auto const flow = builder.internalBuild<3>();
    flow();

    REQUIRE(actual.find('a') != std::string::npos);
    REQUIRE(actual.find('b') != std::string::npos);
    REQUIRE(actual.find('c') != std::string::npos);
    REQUIRE(actual.size() == 3);
}

TEST_CASE("add single parallel 3 with later dependency 1", "[flow]") {
    flow::Builder<> builder;
    actual = "";

    builder.add(a && b && c);
    builder.add(c >> a);

    auto const flow = builder.internalBuild<3>();
    flow();

    REQUIRE(actual.find('a') != std::string::npos);
    REQUIRE(actual.find('b') != std::string::npos);
    REQUIRE(actual.find('c') != std::string::npos);
    REQUIRE(actual.find('c') < actual.find('a'));
    REQUIRE(actual.size() == 3);
}

TEST_CASE("add single parallel 3 with later dependency 2", "[flow]") {
    flow::Builder<> builder;
    actual = "";

    builder.add(a && b && c);
    builder.add(a >> c);

    auto const flow = builder.internalBuild<3>();
    flow();

    REQUIRE(actual.find('a') != std::string::npos);
    REQUIRE(actual.find('b') != std::string::npos);
    REQUIRE(actual.find('c') != std::string::npos);
    REQUIRE(actual.find('a') < actual.find('c'));
    REQUIRE(actual.size() == 3);
}

TEST_CASE("add parallel rhs", "[flow]") {
    flow::Builder<> builder;
    actual = "";

    builder.add(a >> (b && c));

    auto const flow = builder.internalBuild<3>();
    flow();

    REQUIRE(actual.find('a') != std::string::npos);
    REQUIRE(actual.find('b') != std::string::npos);
    REQUIRE(actual.find('c') != std::string::npos);
    REQUIRE(actual.find('a') < actual.find('b'));
    REQUIRE(actual.find('a') < actual.find('c'));
    REQUIRE(actual.size() == 3);
}

TEST_CASE("add parallel lhs", "[flow]") {
    flow::Builder<> builder;
    actual = "";

    builder.add((a && b) >> c);

    auto const flow = builder.internalBuild<3>();
    flow();

    REQUIRE(actual.find('a') != std::string::npos);
    REQUIRE(actual.find('b') != std::string::npos);
    REQUIRE(actual.find('c') != std::string::npos);
    REQUIRE(actual.find('a') < actual.find('c'));
    REQUIRE(actual.find('b') < actual.find('c'));
    REQUIRE(actual.size() == 3);
}

TEST_CASE("add parallel in the middle", "[flow]") {
    flow::Builder<decltype("MiddleParallelFlow"_sc)> builder;
    actual = "";

    builder.add(a >> (b && c) >> d);

    auto const flow = builder.internalBuild<4>();
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
    flow::Builder<> builder;
    actual = "";

    builder.add((a >> b) && c);

    auto const flow = builder.internalBuild<3>();
    flow();

    REQUIRE(actual.find('a') != std::string::npos);
    REQUIRE(actual.find('b') != std::string::npos);
    REQUIRE(actual.find('c') != std::string::npos);

    REQUIRE(actual.find('a') < actual.find('b'));

    REQUIRE(actual.size() == 3);
}

TEST_CASE("add dependency rhs", "[flow]") {
    flow::Builder<> builder;
    actual = "";

    builder.add(a && (b >> c));

    auto const flow = builder.internalBuild<3>();
    flow();

    REQUIRE(actual.find('a') != std::string::npos);
    REQUIRE(actual.find('b') != std::string::npos);
    REQUIRE(actual.find('c') != std::string::npos);

    REQUIRE(actual.find('b') < actual.find('c'));

    REQUIRE(actual.size() == 3);
}

TEST_CASE("add single combination 2", "[flow]") {
    flow::Builder<> builder;
    actual = "";

    builder.add(a, b);

    auto const flow = builder.internalBuild<2>();
    flow();

    REQUIRE(actual.find('a') != std::string::npos);
    REQUIRE(actual.find('b') != std::string::npos);

    REQUIRE(actual.size() == 2);
}
