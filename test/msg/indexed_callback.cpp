#include <match/ops.hpp>
#include <msg/field_matchers.hpp>
#include <msg/indexed_callback.hpp>
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
using msg_t = stdx::tuple<int, char>;

template <typename T, T Value> struct test_m {
    using is_matcher = void;

    [[nodiscard]] constexpr auto operator()(msg_t const &msg) const -> bool {
        return stdx::get<T>(msg) == Value;
    }
    [[nodiscard]] constexpr static auto describe() { return ""_sc; }
    [[nodiscard]] constexpr auto describe_match(auto const &) const {
        return ""_sc;
    }

  private:
    [[nodiscard]] friend constexpr auto tag_invoke(msg::build_index_t,
                                                   test_m const &,
                                                   stdx::callable auto const &f,
                                                   std::size_t &idx) {
        f.template operator()<T>(idx, Value);
    }

    template <typename... Fields>
    [[nodiscard]] friend constexpr auto
    tag_invoke(msg::remove_terms_t, test_m const &m,
               std::type_identity<Fields>...) -> match::matcher auto {
        if constexpr ((std::is_same_v<T, Fields> or ...)) {
            return match::always;
        } else {
            return m;
        }
    }
};
} // namespace

TEST_CASE("callback matches trivially", "[indexed_callback]") {
    constexpr auto cb = msg::indexed_callback(""_sc, test_m<int, 0>{}, [] {});
    CHECK(cb.matcher(msg_t{}));
    CHECK(not cb.matcher(msg_t{1, 'a'}));
}

TEST_CASE("callback with compound match", "[indexed_callback]") {
    constexpr auto cb = msg::indexed_callback(
        ""_sc,
        test_m<int, 0>{} and (test_m<char, 'a'>{} or test_m<char, 'b'>{}),
        [] {});
    CHECK(not cb.matcher(msg_t{}));
    CHECK(cb.matcher(msg_t{0, 'a'}));
    CHECK(cb.matcher(msg_t{0, 'b'}));
}

TEST_CASE("callback is sum of products", "[indexed_callback]") {
    constexpr auto cb = msg::indexed_callback(
        ""_sc,
        test_m<int, 0>{} and (test_m<char, 'a'>{} or test_m<char, 'b'>{}),
        [] {});
    static_assert(
        std::is_same_v<
            decltype(cb.matcher),
            match::or_t<match::and_t<test_m<int, 0>, test_m<char, 'a'>>,
                        match::and_t<test_m<int, 0>, test_m<char, 'b'>>>>);
}

TEST_CASE("index a callback", "[indexed_callback]") {
    constexpr auto cb = msg::indexed_callback(
        ""_sc,
        test_m<int, 0>{} and (test_m<char, 'a'>{} or test_m<char, 'b'>{}),
        [] {});

    auto intfield_index = std::unordered_map<int, std::vector<std::size_t>>{};
    auto charfield_index = std::unordered_map<char, std::vector<std::size_t>>{};

    std::size_t idx{};
    msg::build_index(
        cb.matcher,
        [&]<typename T>(std::size_t cb_idx, T value) {
            if constexpr (std::is_same_v<T, int>) {
                intfield_index[value].push_back(cb_idx);
            } else {
                charfield_index[value].push_back(cb_idx);
            }
        },
        idx);
    CHECK(idx == 1);
    REQUIRE(intfield_index.size() == 1);
    REQUIRE(charfield_index.size() == 2);
    CHECK(intfield_index[0] == std::vector<std::size_t>{0, 1});
    CHECK(charfield_index['a'] == std::vector<std::size_t>{0});
    CHECK(charfield_index['b'] == std::vector<std::size_t>{1});
}

TEST_CASE("remove an indexed term from a callback", "[indexed_callback]") {
    constexpr auto cb = msg::indexed_callback(
        ""_sc,
        test_m<int, 0>{} and (test_m<char, 'a'>{} or test_m<char, 'b'>{}),
        [] {});

    constexpr auto sut = msg::remove_match_terms<int>(cb);
    static_assert(
        std::is_same_v<decltype(sut.matcher),
                       match::or_t<test_m<char, 'a'>, test_m<char, 'b'>>>);
}

TEST_CASE("remove multiple indexed terms from a callback",
          "[indexed_callback]") {
    constexpr auto cb = msg::indexed_callback(
        ""_sc,
        test_m<int, 0>{} and (test_m<char, 'a'>{} or test_m<char, 'b'>{}),
        [] {});

    constexpr auto sut = msg::remove_match_terms<int, char>(cb);
    static_assert(std::is_same_v<decltype(sut.matcher), match::always_t>);
}
