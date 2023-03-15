#include <cib/detail/meta.hpp>
#include <cib/detail/type_list.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

template <unsigned int Value>
struct int_t : public std::integral_constant<unsigned int, Value> {};

TEST_CASE("join two type lists", "[meta]") {
    using ListA = cib::detail::type_list<int_t<0>, int_t<1>, int_t<2>>;
    using ListB = cib::detail::type_list<int_t<3>, int_t<4>, int_t<5>>;

    constexpr auto cat = type_list_cat(ListA{}, ListB{});

    static_assert(cib::detail::is_same_v<
                  decltype(cat),
                  cib::detail::type_list<int_t<0>, int_t<1>, int_t<2>, int_t<3>,
                                         int_t<4>, int_t<5>>>);
}

TEST_CASE("join two type lists with an empty list", "[meta]") {
    using ListA = cib::detail::type_list<int_t<0>>;
    using ListB = cib::detail::type_list<>;

    [[maybe_unused]] constexpr auto cat = type_list_cat(ListA{}, ListB{});

    static_assert(cib::detail::is_same_v<decltype(cat),
                                         cib::detail::type_list<int_t<0>>>);
}
