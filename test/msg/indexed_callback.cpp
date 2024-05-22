#include <match/ops.hpp>
#include <msg/callback.hpp>
#include <msg/detail/indexed_builder_common.hpp>
#include <msg/detail/separate_sum_terms.hpp>
#include <msg/field_matchers.hpp>
#include <msg/message.hpp>
#include <sc/string_constant.hpp>

#include <stdx/concepts.hpp>
#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
using namespace msg;
using int_f =
    msg::field<"int", std::int32_t>::located<msg::at{0_dw, 31_msb, 0_lsb}>;
using char_f =
    msg::field<"char", char32_t>::located<msg::at{1_dw, 31_msb, 0_lsb}>;
using msg_defn = msg::message<"msg", int_f, char_f>;
} // namespace

TEST_CASE("callback matches trivially", "[indexed_callback]") {
    constexpr auto cb =
        msg::callback<"", msg_defn>(msg::equal_to<int_f, 0>, [](auto) {});
    CHECK(cb.matcher(msg_defn::owner_t{}));
    CHECK(
        not cb.matcher(msg_defn::owner_t{"int"_field = 1, "char"_field = 'a'}));
}

TEST_CASE("callback with compound match", "[indexed_callback]") {
    constexpr auto cb = msg::callback<"", msg_defn>(
        msg::equal_to<int_f, 0> and msg::in<char_f, 'a', 'b'>, [](auto) {});
    CHECK(not cb.matcher(msg_defn::owner_t{}));
    CHECK(cb.matcher(msg_defn::owner_t{"int"_field = 0, "char"_field = 'a'}));
    CHECK(cb.matcher(msg_defn::owner_t{"int"_field = 0, "char"_field = 'b'}));
}

TEST_CASE("callback is sum of products", "[indexed_callback]") {
    constexpr auto cb = msg::callback<"", msg_defn>(
        msg::equal_to<int_f, 0> and msg::in<char_f, 'a', 'b'>, [](auto) {});
    static_assert(
        std::is_same_v<
            decltype(cb.matcher),
            match::or_t<
                match::and_t<equal_to_t<int_f, 0>, equal_to_t<char_f, 'a'>>,
                match::and_t<equal_to_t<int_f, 0>, equal_to_t<char_f, 'b'>>>>);
}

TEST_CASE("index a callback", "[indexed_callback]") {
    constexpr auto cb = msg::callback<"", msg_defn>(
        msg::equal_to<int_f, 0> and msg::in<char_f, 'a', 'b'>, [](auto) {});

    auto intfield_index =
        std::unordered_map<std::int32_t, std::vector<std::size_t>>{};
    auto charfield_index =
        std::unordered_map<char32_t, std::vector<std::size_t>>{};

    msg::index_terms(
        cb.matcher,
        [&]<typename T>(std::size_t cb_idx, auto value) {
            if constexpr (std::is_same_v<T, int_f>) {
                intfield_index[value].push_back(cb_idx);
            } else {
                charfield_index[value].push_back(cb_idx);
            }
        },
        std::size_t{});
    REQUIRE(intfield_index.size() == 1);
    REQUIRE(charfield_index.size() == 2);
    CHECK(intfield_index[0] == std::vector<std::size_t>{0, 0});
    CHECK(charfield_index['a'] == std::vector<std::size_t>{0});
    CHECK(charfield_index['b'] == std::vector<std::size_t>{0});
}

TEST_CASE("index not terms in a callback", "[indexed_callback]") {
    constexpr auto cb = msg::callback<"", msg_defn>(
        not msg::equal_to<int_f, 0> and msg::in<char_f, 'a', 'b'>, [](auto) {});

    auto intfield_index =
        std::unordered_map<std::int32_t, std::vector<std::size_t>>{};
    auto charfield_index =
        std::unordered_map<char32_t, std::vector<std::size_t>>{};

    msg::index_not_terms(
        cb.matcher,
        [&]<typename T>(std::size_t cb_idx, auto value) {
            if constexpr (std::is_same_v<T, int_f>) {
                intfield_index[value].push_back(cb_idx);
            } else {
                charfield_index[value].push_back(cb_idx);
            }
        },
        std::size_t{});
    REQUIRE(intfield_index.size() == 1);
    CHECK(charfield_index.empty());
    CHECK(intfield_index[0] == std::vector<std::size_t>{0, 0});
}

TEST_CASE("remove an indexed term from a callback", "[indexed_callback]") {
    constexpr auto cb = msg::callback<"", msg_defn>(
        msg::equal_to<int_f, 0> and msg::in<char_f, 'a', 'b'>, [](auto) {});

    constexpr auto sut = msg::remove_match_terms<int_f>(cb);
    static_assert(
        std::is_same_v<
            decltype(sut.matcher),
            match::or_t<equal_to_t<char_f, 'a'>, equal_to_t<char_f, 'b'>>>);
}

TEST_CASE("remove multiple indexed terms from a callback",
          "[indexed_callback]") {
    constexpr auto cb = msg::callback<"", msg_defn>(
        msg::equal_to<int_f, 0> and msg::in<char_f, 'a', 'b'>, [](auto) {});

    constexpr auto sut = msg::remove_match_terms<int_f, char_f>(cb);
    static_assert(std::is_same_v<decltype(sut.matcher), match::always_t>);
}

namespace {
int called{};
}

TEST_CASE("separate sum terms in a callback", "[indexed_callback]") {
    called = 0;
    constexpr auto cb =
        msg::callback<"", msg_defn>(msg::in<int_f, 0, 1>, []() { ++called; });
    CHECK(cb.matcher(msg_defn::owner_t{}));

    auto sut = msg::separate_sum_terms(cb);
    static_assert(stdx::is_specialization_of_v<decltype(sut), stdx::tuple>);
    static_assert(sut.size() == 2);

    auto const &cb1 = stdx::get<0>(sut);
    auto const &cb2 = stdx::get<1>(sut);

    CHECK(cb1.matcher(msg_defn::owner_t{}));
    CHECK(not cb2.matcher(msg_defn::owner_t{}));
    cb1.callable();
    CHECK(called == 1);

    CHECK(not cb1.matcher(
        msg_defn::owner_t{"int"_field = 1, "char"_field = 'a'}));
    CHECK(cb2.matcher(msg_defn::owner_t{"int"_field = 1, "char"_field = 'a'}));
    cb2.callable();
    CHECK(called == 2);
}
