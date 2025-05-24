#include <log/env.hpp>
#include <log/level.hpp>
#include <log/log.hpp>
#include <log/module.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

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
    STATIC_REQUIRE(custom(cib_log_env_t{}) == 42);
    CIB_LOG_ENV(custom, 1);
    STATIC_REQUIRE(custom(cib_log_env_t{}) == 1);
    {
        CIB_LOG_ENV(custom, 2);
        STATIC_REQUIRE(custom(cib_log_env_t{}) == 2);
    }
}

TEST_CASE("supplement environment", "[log_env]") {
    CIB_LOG_ENV(custom, 1);
    STATIC_REQUIRE(custom(cib_log_env_t{}) == 1);
    {
        using namespace stdx::literals;
        CIB_LOG_ENV(logging::get_module, "hello");
        STATIC_REQUIRE(custom(cib_log_env_t{}) == 1);
        STATIC_REQUIRE(logging::get_module(cib_log_env_t{}) == "hello"_cts);
    }
}

TEST_CASE("multi-value environment", "[log_env]") {
    CIB_LOG_ENV(custom, 1, logging::get_module, "hello");

    using namespace stdx::literals;
    STATIC_REQUIRE(custom(cib_log_env_t{}) == 1);
    STATIC_REQUIRE(logging::get_module(cib_log_env_t{}) == "hello"_cts);
}

namespace {
auto logged_modules = std::vector<std::string>{};

struct log_handler {
    template <typename Env, typename FilenameStringType,
              typename LineNumberType, typename MsgType>
    auto log(FilenameStringType, LineNumberType, MsgType const &) -> void {
        using namespace stdx::literals;
        logged_modules.emplace_back(
            std::string_view{logging::get_module(Env{})});
    }
};

struct log_config {
    log_handler logger;
};
} // namespace

template <> inline auto logging::config<> = log_config{};

TEST_CASE("with-scoped environment", "[log_env]") {
    logged_modules.clear();

    CIB_WITH_LOG_ENV(logging::get_module, "with") { CIB_INFO(""); }
    REQUIRE(logged_modules.size() == 1);
    CHECK(logged_modules[0] == "with");

    CIB_INFO("");
    REQUIRE(logged_modules.size() == 2);
    CHECK(logged_modules[1] == "default");
}
