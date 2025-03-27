#include <log/log.hpp>

#include <stdx/ct_string.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

TEST_CASE("default log module id", "[module_id]") {
    using namespace stdx::literals;
    static_assert(logging::get_module(cib_log_env_t{}) == "default"_cts);
}

namespace ns {
CIB_LOG_MODULE("ns");

TEST_CASE("log module id overridden at namespace scope", "[module_id]") {
    using namespace stdx::literals;
    static_assert(logging::get_module(cib_log_env_t{}) == "ns"_cts);
}
} // namespace ns

struct S {
    template <typename = void> constexpr static auto test() {
        using namespace stdx::literals;
        static_assert(logging::get_module(cib_log_env_t{}) == "S"_cts);
    }

    CIB_LOG_MODULE("S");
};

TEST_CASE("log module id overridden at class scope", "[module_id]") {
    S::test();
}

template <typename = void> constexpr static auto func_test() {
    CIB_LOG_MODULE("fn");
    using namespace stdx::literals;
    static_assert(logging::get_module(cib_log_env_t{}) == "fn"_cts);
}

TEST_CASE("log module id overridden at function scope", "[module_id]") {
    func_test();
}

TEST_CASE("log module id overridden at statement scope", "[module_id]") {
    CIB_LOG_MODULE("wrong");

    {
        CIB_LOG_MODULE("statement");
        using namespace stdx::literals;
        static_assert(logging::get_module(cib_log_env_t{}) == "statement"_cts);
    }
}
