#include <flow/flow.hpp>
#include <log/level.hpp>
#include <nexus/config.hpp>
#include <nexus/nexus.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace {
using namespace flow::literals;

template <auto... Cs> struct wrapper {
    struct inner {
        constexpr static auto config = cib::config(Cs...);
    };
    constexpr static auto n = cib::nexus<inner>{};

    wrapper() { n.init(); }

    template <typename S> static auto run() -> void { n.template service<S>(); }
};

std::vector<logging::level> log_calls{};

template <typename S, auto... Cs>
constexpr auto run_flow = []() -> void {
    log_calls.clear();
    wrapper<Cs...>::template run<S>();
};

struct TestFlowA : public flow::service<"A"> {};
struct TestFlowB : public flow::service<"B"> {};
struct TestFlowC : public flow::service<"C"> {};

constexpr auto msA = flow::milestone<"msA">();
constexpr auto msB = flow::milestone<"msB">();

struct log_config {
    struct log_handler {
        template <typename Env, typename FilenameStringType,
                  typename LineNumberType, typename MsgType>
        auto log(FilenameStringType, LineNumberType, MsgType const &) -> void {
            log_calls.push_back(logging::get_level(Env{}));
        }
    };
    log_handler logger;
};
} // namespace

template <> inline auto logging::config<> = log_config{};

using user1_log_env =
    stdx::extend_env_t<flow::default_log_env, logging::get_level,
                       logging::level::USER1>;

using user2_log_env =
    stdx::extend_env_t<flow::default_log_env, logging::get_level,
                       logging::level::USER2>;

using info_log_env =
    stdx::extend_env_t<flow::default_log_env, logging::get_level,
                       logging::level::INFO>;

template <> constexpr auto flow::log_env<"default"> = info_log_env{};

TEST_CASE("override default log level", "[flow_custom_log_levels]") {
    run_flow<TestFlowA, cib::exports<TestFlowA>,
             cib::extend<TestFlowA>(*msA)>();

    REQUIRE(not log_calls.empty());
    std::for_each(std::begin(log_calls), std::end(log_calls),
                  [](auto level) { CHECK(level == logging::level::INFO); });
}

template <> constexpr auto flow::log_env<"B"> = user1_log_env{};
template <> constexpr auto flow::log_env<"msB"> = user2_log_env{};

TEST_CASE("override log level by name", "[flow_custom_log_levels]") {
    run_flow<TestFlowB, cib::exports<TestFlowB>,
             cib::extend<TestFlowB>(*msB)>();

    REQUIRE(log_calls.size() == 3);
    CHECK(log_calls[0] == logging::level::USER1);
    CHECK(log_calls[1] == logging::level::USER2);
    CHECK(log_calls[2] == logging::level::USER1);
}

template <> constexpr auto flow::log_env<"C"> = user1_log_env{};

TEST_CASE("default log spec for step will use overridden log spec for flow",
          "[flow_custom_log_levels]") {
    run_flow<TestFlowC, cib::exports<TestFlowC>,
             cib::extend<TestFlowC>(*msA)>();

    REQUIRE(log_calls.size() == 3);
    std::for_each(std::begin(log_calls), std::end(log_calls),
                  [](auto level) { CHECK(level == logging::level::USER1); });
}
