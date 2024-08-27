#include <flow/flow.hpp>
#include <flow/graphviz_builder.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {
auto actual = std::string{};

constexpr auto milestone0 = flow::milestone<"milestone0">();
constexpr auto milestone1 = flow::milestone<"milestone1">();

constexpr auto a = flow::action<"a">([] { actual += "a"; });
constexpr auto b = flow::action<"b">([] { actual += "b"; });
constexpr auto c = flow::action<"c">([] { actual += "c"; });
constexpr auto d = flow::action<"d">([] { actual += "d"; });

using builder = flow::graph_builder<flow::impl>;
} // namespace

struct empty_flow {
    constexpr static auto value = flow::graph<>{};
};
TEST_CASE("build and run empty flow", "[graph_builder]") {
    constexpr auto f = builder::render<empty_flow>();
    f();
}

struct single_action {
    constexpr static auto value = flow::graph<>{}.add(a);
};
TEST_CASE("add single action", "[graph_builder]") {
    actual.clear();
    constexpr auto f = builder::render<single_action>();
    f();
    CHECK(actual == "a");
}

struct two_milestone_linear_before {
    constexpr static auto value = flow::graph<>{}.add(a >> milestone0);
};
TEST_CASE("two milestone linear before dependency", "[graph_builder]") {
    actual.clear();
    constexpr auto f = builder::render<two_milestone_linear_before>();
    f();
    CHECK(actual == "a");
}

struct actions_get_executed_once {
    constexpr static auto value = flow::graph<>{}
                                      .add(a >> milestone0)
                                      .add(a >> milestone1)
                                      .add(milestone0 >> milestone1);
};
TEST_CASE("actions get executed once", "[graph_builder]") {
    actual.clear();
    constexpr auto f = builder::render<actions_get_executed_once>();
    f();
    CHECK(actual == "a");
}

struct two_milestone_linear_after_dependency {
    constexpr static auto value = flow::graph<>{}
                                      .add(a >> milestone0)
                                      .add(milestone0 >> milestone1)
                                      .add(milestone0 >> b >> milestone1);
};
TEST_CASE("two milestone linear after dependency", "[graph_builder]") {
    actual.clear();
    constexpr auto f = builder::render<two_milestone_linear_after_dependency>();
    f();
    CHECK(actual == "ab");
}

struct three_milestone_linear_before_and_after_dependency {
    constexpr static auto value = flow::graph<>{}.add(a >> b >> c);
};
TEST_CASE("three milestone linear before and after dependency",
          "[graph_builder]") {
    actual.clear();
    constexpr auto f =
        builder::render<three_milestone_linear_before_and_after_dependency>();
    f();
    CHECK(actual == "abc");
}

struct just_two_actions_in_order {
    constexpr static auto value = flow::graph<>{}.add(a >> b);
};
TEST_CASE("just two actions in order", "[graph_builder]") {
    actual.clear();
    constexpr auto f = builder::render<just_two_actions_in_order>();
    f();
    CHECK(actual == "ab");
}

struct insert_action_between_two_actions {
    constexpr static auto value = flow::graph<>{}.add(a >> c).add(a >> b >> c);
};
TEST_CASE("insert action between two actions", "[graph_builder]") {
    actual.clear();
    constexpr auto f = builder::render<insert_action_between_two_actions>();
    f();
    CHECK(actual == "abc");
}

struct add_single_parallel_2 {
    constexpr static auto value = flow::graph<>{}.add(a && b);
};
TEST_CASE("add single parallel 2", "[graph_builder]") {
    actual.clear();
    constexpr auto f = builder::render<add_single_parallel_2>();
    f();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.size() == 2);
}

struct add_single_parallel_3 {
    constexpr static auto value = flow::graph<>{}.add(a && b && c);
};
TEST_CASE("add single parallel 3", "[graph_builder]") {
    actual.clear();
    constexpr auto f = builder::render<add_single_parallel_3>();
    f();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.size() == 3);
}

struct add_single_parallel_3_with_later_dependency_1 {
    constexpr static auto value = flow::graph<>{}.add(a && b && c).add(c >> a);
};
TEST_CASE("add single parallel 3 with later dependency 1", "[graph_builder]") {
    actual.clear();
    constexpr auto f =
        builder::render<add_single_parallel_3_with_later_dependency_1>();
    f();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('c') < actual.find('a'));
    CHECK(actual.size() == 3);
}

struct add_single_parallel_3_with_later_dependency_2 {
    constexpr static auto value = flow::graph<>{}.add(a && b && c).add(a >> c);
};
TEST_CASE("add single parallel 3 with later dependency 2", "[graph_builder]") {
    actual.clear();
    constexpr auto f =
        builder::render<add_single_parallel_3_with_later_dependency_2>();
    f();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('a') < actual.find('c'));
    CHECK(actual.size() == 3);
}

struct add_parallel_rhs {
    constexpr static auto value = flow::graph<>{}.add(a >> (b && c));
};
TEST_CASE("add parallel rhs", "[graph_builder]") {
    actual.clear();
    constexpr auto f = builder::render<add_parallel_rhs>();
    f();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('a') < actual.find('b'));
    CHECK(actual.find('a') < actual.find('c'));
    CHECK(actual.size() == 3);
}

struct add_parallel_lhs {
    constexpr static auto value = flow::graph<>{}.add((a && b) >> c);
};
TEST_CASE("add parallel lhs", "[graph_builder]") {
    actual.clear();
    constexpr auto f = builder::render<add_parallel_lhs>();
    f();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('a') < actual.find('c'));
    CHECK(actual.find('b') < actual.find('c'));
    CHECK(actual.size() == 3);
}

struct add_parallel_in_the_middle {
    constexpr static auto value = flow::graph<>{}.add(a >> (b && c) >> d);
};
TEST_CASE("add parallel in the middle", "[graph_builder]") {
    actual.clear();
    constexpr auto f = builder::render<add_parallel_in_the_middle>();
    f();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);
    CHECK(actual.find('d') != std::string::npos);

    CHECK(actual.find('a') < actual.find('b'));
    CHECK(actual.find('a') < actual.find('c'));

    CHECK(actual.find('b') < actual.find('d'));
    CHECK(actual.find('c') < actual.find('d'));

    CHECK(actual.size() == 4);
}

struct add_dependency_lhs {
    constexpr static auto value = flow::graph<>{}.add((a >> b) && c);
};
TEST_CASE("add dependency lhs", "[graph_builder]") {
    actual.clear();
    constexpr auto f = builder::render<add_dependency_lhs>();
    f();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);

    CHECK(actual.find('a') < actual.find('b'));

    CHECK(actual.size() == 3);
}

struct add_dependency_rhs {
    constexpr static auto value = flow::graph<>{}.add(a && (b >> c));
};
TEST_CASE("add dependency rhs", "[graph_builder]") {
    actual.clear();
    constexpr auto f = builder::render<add_dependency_rhs>();
    f();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
    CHECK(actual.find('c') != std::string::npos);

    CHECK(actual.find('b') < actual.find('c'));

    CHECK(actual.size() == 3);
}

TEST_CASE("alternate builder", "[graph_builder]") {
    using alt_builder = flow::graphviz_builder;
    auto g = flow::graph<"debug">{}.add(a && (b >> c));
    auto const flow = alt_builder::build(g);
    auto expected = std::string{
        R"__debug__(digraph debug {
start -> a
start -> b
b -> c
a -> end
c -> end
})__debug__"};
    CHECK(flow == expected);
}
