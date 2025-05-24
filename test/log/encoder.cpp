#include <log/catalog/encoder.hpp>

#include <stdx/concepts.hpp>
#include <stdx/ct_format.hpp>
#include <stdx/span.hpp>

#include <conc/concurrency.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace {
constexpr string_id test_string_id = 42u;
constexpr module_id test_module_id = 17u;
} // namespace

template <typename StringType> auto catalog() -> string_id {
    return test_string_id;
}

template <typename StringType> auto module() -> module_id {
    return test_module_id;
}

namespace {
struct [[nodiscard]] test_critical_section {
    test_critical_section() { ++count; }
    ~test_critical_section() { ++count; }
    static inline int count = 0;
};

struct test_conc_policy {
    template <typename = void, stdx::invocable F, stdx::predicate... Pred>
        requires(sizeof...(Pred) < 2)
    static inline auto call_in_critical_section(F &&f, Pred &&...pred)
        -> decltype(std::forward<F>(f)()) {
        while (true) {
            [[maybe_unused]] test_critical_section cs{};
            if ((... and pred())) {
                return std::forward<F>(f)();
            }
        }
    }
};

[[maybe_unused]] constexpr auto expected_short32_header() -> std::uint32_t {
    return (static_cast<std::uint32_t>(test_string_id) << 4u) | 0x1u;
}

[[maybe_unused]] constexpr auto expected_catalog32_header(logging::level level,
                                                          module_id m)
    -> std::uint32_t {
    return (0x1u << 24u) | (m << 16u) |
           (static_cast<std::uint32_t>(level) << 4u) | 0x3u;
}

[[maybe_unused]] constexpr auto expected_msg_header(logging::level level,
                                                    module_id m, std::size_t sz)
    -> std::uint32_t {
    return sz > 0 ? expected_catalog32_header(level, m)
                  : expected_short32_header();
}

int num_log_args_calls{};

constexpr auto check = [](auto value, auto expected) {
    STATIC_REQUIRE(std::is_same_v<decltype(value), decltype(expected)>);
    CHECK(value == expected);
};

constexpr auto check_at = [](auto span, auto dw_idx, std::uint32_t expected) {
    auto idx = dw_idx * sizeof(std::uint32_t);
    auto sz = std::min(sizeof(std::uint32_t), span.size() - idx);

    std::uint32_t actual{};
    std::memcpy(&actual, &span[idx], sz);
    CHECK(actual == stdx::to_le(expected));
};

template <std::uint32_t... Expected>
constexpr auto check_buffer = [](auto data) {
    auto idx = std::size_t{};
    (check_at(data, idx++, Expected), ...);
};

template <logging::level Level, auto... ExpectedArgs>
struct test_log_args_destination {
    template <typename... Args>
    auto log_by_args(std::uint32_t header, Args... args) {
        constexpr auto Header =
            expected_msg_header(Level, test_module_id, sizeof...(ExpectedArgs));
        CHECK(header == Header);
        (check(args, ExpectedArgs), ...);
        ++num_log_args_calls;
    }
};

template <logging::level Level, typename ModuleId, auto... ExpectedArgs>
struct test_log_buf_destination {
    auto log_by_buf(stdx::span<std::uint8_t const> data) const {
        REQUIRE(data.size() ==
                (sizeof...(ExpectedArgs) + 1) * sizeof(std::uint32_t));
        constexpr auto Header =
            expected_msg_header(Level, test_module_id, sizeof...(ExpectedArgs));
        check_buffer<Header, ExpectedArgs...>(data);
        ++num_log_args_calls;
    }
};

template <std::uint32_t Header, std::uint32_t... ExpectedArgs>
struct test_log_version_destination {
    template <typename... Args>
    auto log_by_args(std::uint32_t header, Args... args) {
        CHECK(header == Header);
        (check(args, ExpectedArgs), ...);
        ++num_log_args_calls;
    }

    auto log_by_buf(stdx::span<std::uint8_t const> data) const {
        check_buffer<Header, ExpectedArgs...>(data);
        ++num_log_args_calls;
    }
};

template <logging::level Level> struct test_log_float_args_destination {
    auto log_by_args(std::uint32_t header, auto, auto arg) {
        constexpr auto Header = expected_msg_header(Level, test_module_id, 1);
        CHECK(header == Header);
        CHECK(stdx::bit_cast<float>(arg) == expected);
        ++num_log_args_calls;
    }
    float expected{};
};

template <logging::level Level> struct test_log_double_args_destination {
    auto log_by_args(std::uint32_t header, auto, std::uint32_t lo,
                     std::uint32_t hi) {
        constexpr auto Header = expected_msg_header(Level, test_module_id, 1);
        CHECK(header == Header);
        std::array arr{lo, hi};
        CHECK(stdx::bit_cast<double>(arr) == expected);
        ++num_log_args_calls;
    }
    double expected{};
};

using log_env = stdx::make_env_t<logging::get_level, logging::level::TRACE>;
} // namespace

template <> inline auto conc::injected_policy<> = test_conc_policy{};

TEST_CASE("argument packing", "[mipi]") {
    using P = logging::default_arg_packer;
    STATIC_REQUIRE(std::same_as<P::pack_as_t<std::int32_t>, std::int32_t>);
    STATIC_REQUIRE(std::same_as<P::pack_as_t<std::uint32_t>, std::uint32_t>);
    STATIC_REQUIRE(std::same_as<P::pack_as_t<std::int64_t>, std::int64_t>);
    STATIC_REQUIRE(std::same_as<P::pack_as_t<std::uint64_t>, std::uint64_t>);
    STATIC_REQUIRE(std::same_as<P::pack_as_t<char>, std::int32_t>);
    STATIC_REQUIRE(std::same_as<P::pack_as_t<unsigned char>, std::uint32_t>);
    STATIC_REQUIRE(std::same_as<P::pack_as_t<float>, std::uint32_t>);
    STATIC_REQUIRE(std::same_as<P::pack_as_t<double>, std::uint64_t>);
}

TEST_CASE("argument encoding", "[mipi]") {
    using P = logging::default_arg_packer;
    STATIC_REQUIRE(
        std::same_as<P::encode_as_t<std::int32_t>, encode_32<std::int32_t>>);
    STATIC_REQUIRE(
        std::same_as<P::encode_as_t<std::uint32_t>, encode_u32<std::uint32_t>>);
    STATIC_REQUIRE(
        std::same_as<P::encode_as_t<std::int64_t>, encode_64<std::int64_t>>);
    STATIC_REQUIRE(
        std::same_as<P::encode_as_t<std::uint64_t>, encode_u64<std::uint64_t>>);
    STATIC_REQUIRE(std::same_as<P::encode_as_t<char>, encode_32<char>>);
    STATIC_REQUIRE(
        std::same_as<P::encode_as_t<unsigned char>, encode_u32<unsigned char>>);
    STATIC_REQUIRE(std::same_as<P::encode_as_t<float>, encode_u32<float>>);
    STATIC_REQUIRE(std::same_as<P::encode_as_t<double>, encode_u64<double>>);
}

TEST_CASE("log zero arguments", "[mipi]") {
    CIB_LOG_ENV(logging::get_level, logging::level::TRACE);
    test_critical_section::count = 0;
    auto cfg = logging::binary::config{
        test_log_args_destination<logging::level::TRACE>{}};
    cfg.logger.log_msg<log_env>(stdx::ct_format<"Hello">());
    CHECK(test_critical_section::count == 2);
}

TEST_CASE("log one integral 32-bit argument", "[mipi]") {
    CIB_LOG_ENV(logging::get_level, logging::level::TRACE);
    test_critical_section::count = 0;
    auto cfg = logging::binary::config{
        test_log_args_destination<logging::level::TRACE, 42u, 17u>{}};
    cfg.logger.log_msg<log_env>(stdx::ct_format<"{}">(17u));
    CHECK(test_critical_section::count == 2);
}

TEST_CASE("log one floating-point 32-bit argument", "[mipi]") {
    CIB_LOG_ENV(logging::get_level, logging::level::TRACE);
    test_critical_section::count = 0;
    auto cfg = logging::binary::config{
        test_log_float_args_destination<logging::level::TRACE>{3.14f}};
    cfg.logger.log_msg<log_env>(stdx::ct_format<"{}">(3.14f));
    CHECK(test_critical_section::count == 2);
}

TEST_CASE("log one 64-bit argument", "[mipi]") {
    CIB_LOG_ENV(logging::get_level, logging::level::TRACE);
    test_critical_section::count = 0;
    auto x = std::uint64_t{0x1234'5678'90ab'cdefull};
    auto cfg = logging::binary::config{
        test_log_args_destination<logging::level::TRACE, 42u, 0x90ab'cdefu,
                                  0x1234'5678u>{}};
    cfg.logger.log_msg<log_env>(stdx::ct_format<"{}">(x));
    CHECK(test_critical_section::count == 2);
}

TEST_CASE("log one floating-point 64-bit argument", "[mipi]") {
    CIB_LOG_ENV(logging::get_level, logging::level::TRACE);
    test_critical_section::count = 0;
    auto cfg = logging::binary::config{
        test_log_double_args_destination<logging::level::TRACE>{3.14}};
    cfg.logger.log_msg<log_env>(stdx::ct_format<"{}">(3.14));
    CHECK(test_critical_section::count == 2);
}

TEST_CASE("log two arguments", "[mipi]") {
    CIB_LOG_ENV(logging::get_level, logging::level::TRACE);
    test_critical_section::count = 0;
    auto cfg = logging::binary::config{
        test_log_args_destination<logging::level::TRACE, 42u, 17u, 18u>{}};
    cfg.logger.log_msg<log_env>(stdx::ct_format<"{} {}">(17u, 18u));
    CHECK(test_critical_section::count == 2);
}

TEST_CASE("log more than two arguments", "[mipi]") {
    CIB_LOG_ENV(logging::get_level, logging::level::TRACE);
    {
        test_critical_section::count = 0;
        auto cfg = logging::binary::config{
            test_log_buf_destination<logging::level::TRACE, log_env, 42u, 17u,
                                     18u, 19u>{}};
        cfg.logger.log_msg<log_env>(stdx::ct_format<"{} {} {}">(17u, 18u, 19u));
        CHECK(test_critical_section::count == 2);
    }
    {
        test_critical_section::count = 0;
        auto cfg = logging::binary::config{
            test_log_buf_destination<logging::level::TRACE, log_env, 42u, 17u,
                                     18u, 97u, 98u>{}};
        cfg.logger.log_msg<log_env>(
            stdx::ct_format<"{} {} {} {}">(17u, 18u, 'a', 'b'));
        CHECK(test_critical_section::count == 2);
    }
}

TEST_CASE("log to multiple destinations", "[mipi]") {
    CIB_LOG_ENV(logging::get_level, logging::level::TRACE);
    test_critical_section::count = 0;
    num_log_args_calls = 0;
    auto cfg = logging::binary::config{
        test_log_args_destination<logging::level::TRACE, 42u, 17u, 18u>{},
        test_log_args_destination<logging::level::TRACE, 42u, 17u, 18u>{}};

    cfg.logger.log_msg<log_env>(stdx::ct_format<"{} {}">(17u, 18u));
    CHECK(test_critical_section::count == 4);
    CHECK(num_log_args_calls == 2);
}

TEST_CASE("log version information (compact32)", "[mipi]") {
    test_critical_section::count = 0;
    num_log_args_calls = 0;
    auto cfg = logging::binary::config{test_log_version_destination<
        0b11'000000'1010'1011'1100'1101'0101'0000u>{}};
    //     3      0    a    b    c    d    5
    cfg.logger.log_version<log_env, 0x3abcd5u>();
    CHECK(test_critical_section::count == 2);
    CHECK(num_log_args_calls == 1);
}

TEST_CASE("log version information (compact64)", "[mipi]") {
    test_critical_section::count = 0;
    num_log_args_calls = 0;
    auto cfg = logging::binary::config{
        test_log_version_destination<0b11'000001'1010'1011'1100'1101'0101'0000u,
                                     // 3      1    a    b    c    d    5
                                     0b1'0010'0011'0100'0101'0110'00u>{}};
    //                                 1    2    3    4    5    6
    cfg.logger.log_version<log_env, 0x1234'563a'bcd5u>();
    CHECK(test_critical_section::count == 2);
    CHECK(num_log_args_calls == 1);
}

TEST_CASE("log version information (long with string)", "[mipi]") {
    test_critical_section::count = 0;
    num_log_args_calls = 0;
    auto cfg = logging::binary::config{
        test_log_version_destination<0b10'0000'0000'0000'0010'0000'0000u,
                                     0x4321'000du, 0x5678'8765u, 0x65'68'1234u,
                                     //                             e  h
                                     0x6f'6c'6cu>{}};
    //                                  o  l  l
    cfg.logger.log_version<log_env, 0x1234'5678'8765'4321ull, "hello">();
    CHECK(test_critical_section::count == 2);
    CHECK(num_log_args_calls == 1);
}

template <>
inline auto logging::config<> =
    logging::binary::config{test_log_args_destination<logging::level::TRACE>{}};

TEST_CASE("injection", "[mipi]") {
    test_critical_section::count = 0;
    CIB_TRACE("Hello");
    CHECK(test_critical_section::count == 2);
}

namespace {
int num_catalog_args_calls{};

template <logging::level Level> struct test_catalog_args_destination {
    template <typename... Args>
    auto log_by_args(std::uint32_t header, std::uint32_t id, Args...) {
        constexpr auto Header =
            expected_catalog32_header(Level, test_module_id);
        CHECK(header == Header);
        CHECK(id == test_string_id);
        STATIC_REQUIRE(sizeof...(Args) == 0);
        ++num_catalog_args_calls;
    }
};

struct custom_builder : logging::mipi::default_builder<> {
    template <auto Level, logging::packable... Ts>
    static auto build(string_id id, module_id m, Ts... args) {
        return logging::mipi::builder<logging::mipi::defn::catalog_msg_t,
                                      logging::default_arg_packer>{}
            .template build<Level>(id, m, args...);
    }
};
} // namespace

TEST_CASE("log with overridden builder", "[mipi]") {
    using catalog_env =
        stdx::make_env_t<logging::get_level, logging::level::TRACE,
                         logging::binary::get_builder, custom_builder{}>;

    num_catalog_args_calls = 0;
    auto cfg = logging::binary::config{
        test_catalog_args_destination<logging::level::TRACE>{}};

    cfg.logger.log_msg<catalog_env>(stdx::ct_format<"Hello">());
    CHECK(num_catalog_args_calls == 1);
}
