#include <log/log.hpp>
#include <sc/string_constant.hpp>

#include <stdx/ct_string.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

TEST_CASE("default log module id", "[module_id]") {
    static_assert(std::is_same_v<cib_log_module_id_t, decltype("default"_sc)>);
}

namespace ns {
CIB_LOG_MODULE("ns");

TEST_CASE("log module id overridden at namespace scope", "[module_id]") {
    static_assert(std::is_same_v<cib_log_module_id_t, decltype("ns"_sc)>);
}
} // namespace ns

struct S {
    CIB_LOG_MODULE("S");

    template <typename = void> constexpr static auto test() {
        static_assert(std::is_same_v<cib_log_module_id_t, decltype("S"_sc)>);
    }
};

TEST_CASE("log module id overridden at class scope", "[module_id]") {
    S::test();
}

template <typename = void> constexpr static auto func_test() {
    CIB_LOG_MODULE("fn");
    static_assert(std::is_same_v<cib_log_module_id_t, decltype("fn"_sc)>);
}

TEST_CASE("log module id overridden at function scope", "[module_id]") {
    func_test();
}

TEST_CASE("log module id overridden at statement scope", "[module_id]") {
    CIB_LOG_MODULE("wrong");

    {
        CIB_LOG_MODULE("statement");
        static_assert(
            std::is_same_v<cib_log_module_id_t, decltype("statement"_sc)>);
    }
}
