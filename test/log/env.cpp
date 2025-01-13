#include <log/env.hpp>
#include <log/module.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
[[maybe_unused]] constexpr inline struct custom_t {
    template <typename T>
        requires true // more constrained
    CONSTEVAL auto operator()(T &&t) const
        noexcept(noexcept(std::forward<T>(t).query(std::declval<custom_t>())))
            -> decltype(std::forward<T>(t).query(*this)) {
        return std::forward<T>(t).query(*this);
    }

    CONSTEVAL auto operator()(auto &&) const {
        using namespace stdx::literals;
        return 42;
    }
} custom;
} // namespace

TEST_CASE("override environment", "[log_env]") {
    static_assert(custom(cib_log_env_t{}) == 42);
    CIB_LOG_ENV(custom, 1);
    static_assert(custom(cib_log_env_t{}) == 1);
    {
        CIB_LOG_ENV(custom, 2);
        static_assert(custom(cib_log_env_t{}) == 2);
    }
}

TEST_CASE("supplement environment", "[log_env]") {
    CIB_LOG_ENV(custom, 1);
    static_assert(custom(cib_log_env_t{}) == 1);
    {
        using namespace stdx::literals;
        CIB_LOG_ENV(logging::get_module, "hello");
        static_assert(custom(cib_log_env_t{}) == 1);
        static_assert(logging::get_module(cib_log_env_t{}) == "hello"_ctst);
    }
}

TEST_CASE("multi-value environment", "[log_env]") {
    CIB_LOG_ENV(custom, 1, logging::get_module, "hello");

    using namespace stdx::literals;
    static_assert(custom(cib_log_env_t{}) == 1);
    static_assert(logging::get_module(cib_log_env_t{}) == "hello"_ctst);
}
