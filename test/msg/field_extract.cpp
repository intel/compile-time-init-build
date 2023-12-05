#include <msg/field.hpp>

#include <stdx/type_traits.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>

using namespace msg;

TEST_CASE("single bit", "[field extract]") {
    using F = field<"", std::uint32_t>::located<at{0_dw, 1_msb, 1_lsb}>;
    std::array<std::uint32_t, 1> data{0b010};
    CHECK(0b1u == F::extract(data));
}

TEST_CASE("within one storage element", "[field extract]") {
    using F = field<"", std::uint32_t>::located<at{0_dw, 16_msb, 5_lsb}>;
    std::array<std::uint32_t, 1> data{0b11'1010'1010'1110'0000};
    //                                   ^-------------^
    CHECK(0b1'1010'1010'111u == F::extract(data));
}

TEST_CASE("within one storage element (lsb aligned)", "[field extract]") {
    using F = field<"", std::uint32_t>::located<at{0_dw, 6_msb, 0_lsb}>;
    std::array<std::uint32_t, 1> data{0b1101'0111};
    //                                   ^------^
    CHECK(0b101'0111u == F::extract(data));
}

TEST_CASE("within one storage element (msb aligned)", "[field extract]") {
    using F = field<"", std::uint32_t>::located<at{0_dw, 31_msb, 26_lsb}>;
    std::array<std::uint32_t, 1> data{0b1010'1110u << 24u};
    //                                  ^-----^
    CHECK(0b1010'11u == F::extract(data));
}

TEST_CASE("across two storage elements", "[field extract]") {
    using F = field<"", std::uint32_t>::located<at{0_dw, 37_msb, 26_lsb}>;
    std::array<std::uint32_t, 2> data{0b1010'1110u << 24u, 0b0111'1010u};
    CHECK(0b11'1010'1010'11u == F::extract(data));
}

TEST_CASE("disjoint", "[field extract]") {
    using F = field<"", std::uint32_t>::located<at{0_dw, 5_msb, 3_lsb},
                                                at{0_dw, 11_msb, 9_lsb}>;
    std::array<std::uint32_t, 1> data{0b1'1011'1110'1110u};
    //                                    ^-^    ^--^
    CHECK(0b101'101u == F::extract(data));
}

TEST_CASE("across multiple storage elements", "[field extract]") {
    using F = field<"", std::uint32_t>::located<at{0_dw, 17_msb, 2_lsb}>;
    std::array<std::uint8_t, 3> data{
        0b1010'1110u,
        0b1010'1010u,
        0b111u,
    };
    CHECK(0b11'1010'1010'1010'11u == F::extract(data));
}

TEST_CASE("without dword index", "[field extract]") {
    using F = field<"", std::uint32_t>::located<at{17_msb, 2_lsb}>;
    std::array<std::uint8_t, 3> data{
        0b1010'1110u,
        0b1010'1010u,
        0b111u,
    };
    CHECK(0b11'1010'1010'1010'11u == F::extract(data));
}

TEST_CASE("max field size", "[field extract]") {
    using F = field<"", std::uint64_t>::located<at{65_msb, 2_lsb}>;
    std::array<std::uint8_t, 9> data{
        0b1010'1110u, 0b1010'1010u, 0b1010'1010u, 0b1010'1010u, 0b1010'1010u,
        0b1010'1010u, 0b1010'1010u, 0b1010'1010u, 0b111u,
    };
    CHECK(0xeaaa'aaaa'aaaa'aaabu == F::extract(data));
}

namespace {
enum struct E : std::uint8_t { Value = 0b10101 };
} // namespace

TEST_CASE("enum field type ", "[field extract]") {
    using F = field<"", E>::located<at{0_dw, 5_msb, 1_lsb}>;
    std::array<std::uint8_t, 1> data{0b1110'1011};
    //                                   ^----^
    CHECK(E::Value == F::extract(data));
}
