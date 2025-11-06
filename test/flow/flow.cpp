#include <cib/cib.hpp>
#include <cib/func_decl.hpp>
#include <flow/flow.hpp>
#include <flow/viz_builder.hpp>
#include <log/fmt/logger.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

std::string actual = {};

namespace {
using namespace flow::literals;

constexpr auto a = flow::action<"a">([] { actual += "a"; });
constexpr auto b = flow::action<"b">([] { actual += "b"; });
constexpr auto c = flow::action<"c">([] { actual += "c"; });
constexpr auto d = flow::action<"d">([] { actual += "d"; });

struct TestFlowAlpha : public flow::service<> {};
struct TestFlowBeta : public flow::service<> {};

template <auto... Cs> struct wrapper {
    struct inner {
        constexpr static auto config = cib::config(Cs...);
    };
    constexpr static auto n = cib::nexus<inner>{};

    wrapper() { n.init(); }

    template <typename S> static auto run() -> void { n.template service<S>(); }
};

template <typename S, auto... Cs>
constexpr auto check_flow = [](std::string_view expected) -> void {
    actual.clear();
    wrapper<Cs...>::template run<S>();
    CHECK(actual == expected);
};
} // namespace

template <bool V>
constexpr auto when = cib::runtime_condition<"when">([] { return V; });

TEST_CASE("run empty flow through cib::nexus", "[flow]") {
    check_flow<TestFlowAlpha, cib::exports<TestFlowAlpha>>("");
}

namespace {
struct VizFlow
    : public flow::service_for<
          flow::builder_for<flow::viz_builder<"debug", flow::mermaid>>> {};
struct VizCondConfig {
    constexpr static auto config =
        cib::config(cib::exports<VizFlow>, cib::extend<VizFlow>(*a),
                    when<true>(cib::extend<VizFlow>(a >> *b)));
};
} // namespace

TEST_CASE("visualize flow (mermaid)", "[flow]") {
    using namespace std::string_view_literals;
    cib::nexus<VizCondConfig> nexus{};
    constexpr auto viz = nexus.service<VizFlow>();
    constexpr auto expected =
        R"__debug__(---
title: debug
---
flowchart TD
  _start((start))
  _end((end))
  _a(a)
  _b(b)
  _a --when--> _b
  _start --> _a
  _b --when--> _end
)__debug__"sv;
    STATIC_REQUIRE(viz == expected);
}

TEST_CASE("add single action through cib::nexus", "[flow]") {
    check_flow<TestFlowAlpha, cib::exports<TestFlowAlpha>,
               cib::extend<TestFlowAlpha>(*a)>("a");
}

TEST_CASE("add runtime conditional action through cib::nexus", "[flow]") {
    check_flow<TestFlowAlpha, cib::exports<TestFlowAlpha>,
               when<true>(cib::extend<TestFlowAlpha>(*a))>("a");

    check_flow<TestFlowAlpha, cib::exports<TestFlowAlpha>,
               when<false>(cib::extend<TestFlowAlpha>(*a))>("");
}

TEST_CASE("add multi-level runtime conditional action through cib::nexus",
          "[flow]") {
    auto check = []<bool level0, bool level1>(auto expected) {
        check_flow<TestFlowAlpha, cib::exports<TestFlowAlpha>,
                   when<level0>(when<level1>(cib::extend<TestFlowAlpha>(*a)))>(
            expected);
    };

    check.operator()<true, true>("a");
    check.operator()<true, false>("");
    check.operator()<false, true>("");
    check.operator()<false, false>("");
}

TEST_CASE("dependencies between runtime conditional actions through cib::nexus",
          "[flow]") {
    auto check = []<bool ca, bool cb>(auto expected) {
        check_flow<TestFlowAlpha, cib::exports<TestFlowAlpha>,
                   when<ca>(cib::extend<TestFlowAlpha>(*a)),
                   when<cb>(cib::extend<TestFlowAlpha>(*b)),
                   (when<ca> and when<cb>)(cib::extend<TestFlowAlpha>(a >> b))>(
            expected);
    };

    check.operator()<true, true>("ab");
    check.operator()<true, false>("a");
    check.operator()<false, true>("b");
    check.operator()<false, false>("");
}

TEST_CASE("dependency conditions imply action conditions", "[flow]") {
    auto check = []<bool ca, bool cb, bool cc>(auto expected) {
        check_flow<TestFlowAlpha, cib::exports<TestFlowAlpha>,
                   when<ca>(cib::extend<TestFlowAlpha>(*a)),
                   when<cb>(cib::extend<TestFlowAlpha>(*b)),
                   (when<ca> and when<cb> and
                    when<cc>)(cib::extend<TestFlowAlpha>(a >> b))>(expected);
    };

    check.operator()<true, true, true>("ab");
    check.operator()<true, false, true>("a");
    check.operator()<true, false, false>("a");
    check.operator()<false, true, true>("b");
    check.operator()<false, true, false>("b");
    check.operator()<false, false, false>("");
}

TEST_CASE("dependencies between runtime conditional and always actions through "
          "cib::nexus",
          "[flow]") {
    auto check = []<bool ca, bool cb>(auto expected) {
        check_flow<TestFlowAlpha, cib::exports<TestFlowAlpha>,
                   when<ca>(cib::extend<TestFlowAlpha>(*a >> c)),
                   when<cb>(cib::extend<TestFlowAlpha>(c >> *b)),
                   cib::extend<TestFlowAlpha>(*c)>(expected);
    };

    check.operator()<true, true>("acb");
    check.operator()<true, false>("ac");
    check.operator()<false, true>("cb");
    check.operator()<false, false>("c");
}

TEST_CASE("add par runtime conditional (true) actions through cib::nexus",
          "[flow]") {
    actual.clear();

    auto n0 = wrapper<cib::exports<TestFlowAlpha>,
                      when<true>(cib::extend<TestFlowAlpha>(*a && *b))>{};

    n0.run<TestFlowAlpha>();

    CHECK(actual.find('a') != std::string::npos);
    CHECK(actual.find('b') != std::string::npos);
}

TEST_CASE("add par runtime conditional (false) actions through cib::nexus",
          "[flow]") {
    check_flow<TestFlowAlpha, cib::exports<TestFlowAlpha>,
               when<false>(cib::extend<TestFlowAlpha>(*a && *b))>("");
}

TEST_CASE("add multi action through cib::nexus", "[flow]") {
    actual.clear();

    auto n = wrapper<cib::exports<TestFlowAlpha, TestFlowBeta>,
                     cib::extend<TestFlowAlpha>(*a >> *b),
                     cib::extend<TestFlowBeta>(*c >> *d)>{};

    n.run<TestFlowAlpha>();
    n.run<TestFlowBeta>();
    CHECK(actual == "abcd");
}

TEST_CASE("add multi action through cib::nexus, run through cib::service",
          "[flow]") {
    actual.clear();

    wrapper<cib::exports<TestFlowAlpha, TestFlowBeta>,
            cib::extend<TestFlowAlpha>(*a >> *b),
            cib::extend<TestFlowBeta>(*c >> *d)>{};

    cib::service<TestFlowAlpha>();
    cib::service<TestFlowBeta>();
    CHECK(actual == "abcd");
}
