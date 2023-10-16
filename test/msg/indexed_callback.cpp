#include <match/ops.hpp>
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
constexpr class index_t {
    template <match::matcher M>
    [[nodiscard]] friend constexpr auto tag_invoke(index_t, M const &m,
                                                   stdx::callable auto const &f,
                                                   std::size_t &idx) {
        if constexpr (stdx::is_specialization_of_v<M, match::or_t>) {
            tag_invoke(index_t{}, m.lhs, f, idx);
            ++idx;
            tag_invoke(index_t{}, m.rhs, f, idx);
        } else if constexpr (stdx::is_specialization_of_v<M, match::and_t>) {
            tag_invoke(index_t{}, m.lhs, f, idx);
            tag_invoke(index_t{}, m.rhs, f, idx);
        } else if constexpr (stdx::is_specialization_of_v<M, match::not_t>) {
            tag_invoke(index_t{}, m.m, f, idx);
        } else {
            static_assert(stdx::always_false_v<M>,
                          "Unexpected type while indexing matchers");
        }
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<index_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} test_index{};

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
    [[nodiscard]] friend constexpr auto tag_invoke(index_t, test_m const &,
                                                   stdx::callable auto const &f,
                                                   std::size_t &idx) {
        f.template operator()<T>(idx, Value);
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
    test_index(
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
