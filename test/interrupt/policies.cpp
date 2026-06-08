#include <interrupt/policies.hpp>

#include <stdx/bitset.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("policies get", "[policies]") {
    using policies_t = interrupt::policies<interrupt::clear_status_first>;
    using default_policy_t = interrupt::clear_status_last;
    STATIC_CHECK(
        std::is_same_v<decltype(policies_t::get<interrupt::status_clear_policy,
                                                default_policy_t>()),
                       interrupt::clear_status_first>);
    STATIC_CHECK(std::is_same_v<
                 decltype(policies_t::get<interrupt::required_resources_policy,
                                          default_policy_t>()),
                 default_policy_t>);
}

TEST_CASE("clear status first policy", "[policies]") {
    int call_value{};
    interrupt::clear_status_first::run([&] { ++call_value; },
                                       [&] { call_value *= 2; });
    CHECK(call_value == 2);
}

TEST_CASE("clear status last policy", "[policies]") {
    int call_value{};
    interrupt::clear_status_last::run([&] { ++call_value; },
                                      [&] { call_value *= 2; });
    CHECK(call_value == 1);
}

TEST_CASE("don't clear status policy", "[policies]") {
    int call_value{};
    interrupt::dont_clear_status::run([&] { call_value += 3; },
                                      [&] { ++call_value; });
    CHECK(call_value == 1);
}

TEST_CASE("policy with required resources", "[policies]") {
    using policies_t = interrupt::policies<interrupt::required_resources<int>>;
    using P =
        decltype(policies_t::get<interrupt::required_resources_policy, void>());
    STATIC_CHECK(interrupt::policy<P>);
    STATIC_CHECK(
        std::is_same_v<typename P::resources, interrupt::resource_list<int>>);
}

namespace {
using P = interrupt::dynamic_enable_policy;

template <stdx::ct_string Irq> struct flow;
template <stdx::ct_string Irq> struct resource;

struct grandchild {
    constexpr static auto name = stdx::ct_string{"gc"};
    using name_t = stdx::cts_t<name>;

    constexpr static auto children = stdx::tuple{};
    using resources_t = interrupt::resource_list<resource<name>>;
    using flows_t = stdx::type_list<flow<name>>;
};

struct child_a {
    constexpr static auto name = stdx::ct_string{"ca"};
    using name_t = stdx::cts_t<name>;

    constexpr static auto children = stdx::tuple{grandchild{}};
    using resources_t =
        interrupt::resource_list<resource<"ca1">, resource<"ca2">>;
    using flows_t = stdx::type_list<flow<name>>;
};

struct child_b {
    constexpr static auto name = stdx::ct_string{"cb"};
    using name_t = stdx::cts_t<name>;

    constexpr static auto children = stdx::tuple{};
    using resources_t = interrupt::resource_list<resource<name>>;
    using flows_t = stdx::type_list<flow<"cb1">, flow<"cb2">>;
};

struct parent {
    constexpr static auto name = stdx::ct_string{"p"};
    using name_t = stdx::cts_t<name>;

    constexpr static auto children = stdx::tuple{child_a{}, child_b{}};
    using resources_t = interrupt::resource_list<resource<name>>;
    using flows_t = stdx::type_list<flow<name>>;
};

using name_enables_t = stdx::type_bitset<parent::name_t, child_a::name_t,
                                         child_b::name_t, grandchild::name_t>;
using flow_enables_t = boost::mp11::mp_apply<
    stdx::type_bitset,
    boost::mp11::mp_append<parent::flows_t, child_a::flows_t, child_b::flows_t,
                           grandchild::flows_t>>;

using resource_enables_t = boost::mp11::mp_apply<
    stdx::type_bitset,
    boost::mp11::mp_append<parent::resources_t, child_a::resources_t,
                           child_b::resources_t, grandchild::resources_t>>;
} // namespace

TEST_CASE("dynamic_enable_policy: is_on (all enables)", "[policies]") {
    constexpr auto name_enables = name_enables_t{stdx::all_bits};
    constexpr auto rsrc_enables = resource_enables_t{stdx::all_bits};
    constexpr auto flow_enables = flow_enables_t{stdx::all_bits};
    STATIC_CHECK(P::is_on<parent>(name_enables, rsrc_enables, flow_enables));
    STATIC_CHECK(P::is_on<child_a>(name_enables, rsrc_enables, flow_enables));
    STATIC_CHECK(P::is_on<child_b>(name_enables, rsrc_enables, flow_enables));
    STATIC_CHECK(
        P::is_on<grandchild>(name_enables, rsrc_enables, flow_enables));
}

TEST_CASE("dynamic_enable_policy: is_on (turned off by name)", "[policies]") {
    // everything enabled except parent name
    constexpr auto name_enables =
        name_enables_t{stdx::type_list<child_a::name_t, child_b::name_t,
                                       grandchild::name_t>{}};
    constexpr auto rsrc_enables = resource_enables_t{stdx::all_bits};
    constexpr auto flow_enables = flow_enables_t{stdx::all_bits};
    STATIC_CHECK(
        not P::is_on<parent>(name_enables, rsrc_enables, flow_enables));
}

TEST_CASE("dynamic_enable_policy: is_on (turned off by resource)",
          "[policies]") {
    // everything enabled except parent resource
    constexpr auto name_enables = name_enables_t{stdx::all_bits};
    constexpr auto rsrc_enables =
        resource_enables_t{stdx::type_list<resource<"ca1">, resource<"ca2">,
                                           resource<"cb">, resource<"gc">>{}};
    constexpr auto flow_enables = flow_enables_t{stdx::all_bits};
    STATIC_CHECK(
        not P::is_on<parent>(name_enables, rsrc_enables, flow_enables));
}

TEST_CASE("dynamic_enable_policy: is_on (by flows)", "[policies]") {
    // only parent name enabled; children off
    constexpr auto name_enables =
        name_enables_t{stdx::type_list<parent::name_t>{}};
    constexpr auto rsrc_enables = resource_enables_t{stdx::all_bits};
    constexpr auto flow_enables = flow_enables_t{stdx::all_bits};
    STATIC_CHECK(P::is_on<parent>(name_enables, rsrc_enables, flow_enables));
}

TEST_CASE("dynamic_enable_policy: is_on (by one flow)", "[policies]") {
    // only child_b enabled
    constexpr auto name_enables =
        name_enables_t{stdx::type_list<child_b::name_t>{}};
    constexpr auto rsrc_enables = resource_enables_t{stdx::all_bits};
    // only one of child_b's flows enabled
    constexpr auto flow_enables =
        flow_enables_t{stdx::type_list<flow<"cb1">>{}};
    STATIC_CHECK(P::is_on<child_b>(name_enables, rsrc_enables, flow_enables));
}

TEST_CASE("dynamic_enable_policy: is_on (by children)", "[policies]") {
    constexpr auto name_enables = name_enables_t{
        stdx::type_list<parent::name_t, child_a::name_t, child_b::name_t>{}};
    constexpr auto rsrc_enables = resource_enables_t{stdx::all_bits};
    // parent flow not enabled, but children are
    constexpr auto flow_enables =
        flow_enables_t{stdx::type_list<flow<"ca">, flow<"cb1">>{}};
    STATIC_CHECK(P::is_on<parent>(name_enables, rsrc_enables, flow_enables));
}

TEST_CASE("dynamic_enable_policy: is_on (by one child)", "[policies]") {
    constexpr auto name_enables =
        name_enables_t{stdx::type_list<parent::name_t, child_a::name_t>{}};
    constexpr auto rsrc_enables = resource_enables_t{stdx::all_bits};
    // parent flow not enabled, but child_a's is
    constexpr auto flow_enables = flow_enables_t{stdx::type_list<flow<"ca">>{}};
    STATIC_CHECK(P::is_on<parent>(name_enables, rsrc_enables, flow_enables));
}

TEST_CASE("dynamic_enable_policy: is_on (by grandchild)", "[policies]") {
    constexpr auto name_enables = name_enables_t{
        stdx::type_list<parent::name_t, child_a::name_t, grandchild::name_t>{}};
    constexpr auto rsrc_enables = resource_enables_t{stdx::all_bits};
    // grandchild on => child_a on => parent on
    constexpr auto flow_enables = flow_enables_t{stdx::type_list<flow<"gc">>{}};
    STATIC_CHECK(P::is_on<child_a>(name_enables, rsrc_enables, flow_enables));
    STATIC_CHECK(P::is_on<parent>(name_enables, rsrc_enables, flow_enables));
}

namespace {
struct alt_grandchild {
    constexpr static auto name = stdx::ct_string{"gc"};
    using name_t = stdx::cts_t<name>;

    constexpr static auto children = stdx::tuple{};
    using resources_t = interrupt::resource_list<>;
    using flows_t = stdx::type_list<flow<name>>;
};
} // namespace

TEST_CASE("dynamic_enable_policy: is_on (no resources)", "[policies]") {
    constexpr auto name_enables =
        name_enables_t{stdx::type_list<alt_grandchild::name_t>{}};
    constexpr auto rsrc_enables = resource_enables_t{};
    constexpr auto flow_enables = flow_enables_t{stdx::type_list<flow<"gc">>{}};
    STATIC_CHECK(
        P::is_on<alt_grandchild>(name_enables, rsrc_enables, flow_enables));
}

namespace {
struct alt_parent {
    constexpr static auto name = stdx::ct_string{"p"};
    using name_t = stdx::cts_t<name>;

    constexpr static auto children = stdx::tuple{child_a{}};
    using resources_t = interrupt::resource_list<>;
    using flows_t = stdx::type_list<>;
};
} // namespace

TEST_CASE("dynamic_enable_policy: is_on (parent with no resources/flows)",
          "[policies]") {
    constexpr auto name_enables =
        name_enables_t{stdx::type_list<alt_parent::name_t, child_a::name_t>{}};
    constexpr auto rsrc_enables =
        resource_enables_t{stdx::type_list<resource<"ca1">, resource<"ca2">>{}};
    constexpr auto flow_enables = flow_enables_t{stdx::type_list<flow<"ca">>{}};
    STATIC_CHECK(
        P::is_on<alt_parent>(name_enables, rsrc_enables, flow_enables));
}
