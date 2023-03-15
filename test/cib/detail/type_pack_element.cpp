#include <cib/detail/type_pack_element.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

TEST_CASE("type pack element") {
    static_assert(std::is_same_v<int, cib::detail::type_pack_element<0, int>>);
    static_assert(
        std::is_same_v<int, cib::detail::type_pack_element<0, int, float>>);
    static_assert(
        std::is_same_v<float, cib::detail::type_pack_element<1, int, float>>);
    static_assert(
        std::is_same_v<void,
                       cib::detail::type_pack_element<0, void, void, void>>);
    static_assert(
        std::is_same_v<void,
                       cib::detail::type_pack_element<1, void, void, void>>);
    static_assert(
        std::is_same_v<void,
                       cib::detail::type_pack_element<2, void, void, void>>);
    static_assert(
        std::is_same_v<const int &,
                       cib::detail::type_pack_element<0, const int &, void *>>);
    static_assert(
        std::is_same_v<void *,
                       cib::detail::type_pack_element<1, const int &, void *>>);
}
