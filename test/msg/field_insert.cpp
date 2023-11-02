#include <msg/field.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>

using namespace msg;

TEST_CASE("single bit", "[field insert]") {
    using F = field<"", std::uint32_t>::located<at{0_dw, 1_msb, 1_lsb}>;
    std::array<std::uint32_t, 1> data{};
    F::insert(data, 1u);
    CHECK(0b010u == data[0]);
}

TEST_CASE("within one storage element", "[field insert]") {
    using F = field<"", std::uint32_t>::located<at{0_dw, 16_msb, 5_lsb}>;
    std::array<std::uint32_t, 1> data{0b10'0000'0000'0001'0000};
    //                                   ^-------------^
    F::insert(data, 0b1'1010'1010'111u);
    CHECK(0b11'1010'1010'1111'0000 == data[0]);
}

TEST_CASE("within one storage element (lsb aligned)", "[field insert]") {
    using F = field<"", std::uint32_t>::located<at{0_dw, 6_msb, 0_lsb}>;
    std::array<std::uint32_t, 1> data{0b1000'0000};
    //                                   ^------^
    F::insert(data, 0b101'0111u);
    CHECK(0b1101'0111u == data[0]);
}

TEST_CASE("within one storage element (msb aligned)", "[field insert]") {
    using F = field<"", std::uint32_t>::located<at{0_dw, 31_msb, 26_lsb}>;
    std::array<std::uint32_t, 1> data{0b0000'0010u << 24u};
    //                                  ^-----^
    F::insert(data, 0b1010'11u);
    CHECK(0b1010'1110u << 24u == data[0]);
}

TEST_CASE("across two storage elements", "[field insert]") {
    using F = field<"", std::uint32_t>::located<at{0_dw, 37_msb, 26_lsb}>;
    std::array<std::uint32_t, 2> data{0b0000'0010u << 24u, 0b0100'0000u};
    F::insert(data, 0b11'1010'1010'11u);
    CHECK(0b1010'1110u << 24u == data[0]);
    CHECK(0b0111'1010u == data[1]);
}

TEST_CASE("disjoint", "[field insert]") {
    using F = field<"", std::uint32_t>::located<at{0_dw, 5_msb, 3_lsb},
                                                at{0_dw, 11_msb, 9_lsb}>;
    std::array<std::uint32_t, 1> data{0b1'0001'1100'0110u};
    //                                    ^-^    ^--^
    F::insert(data, 0b101'101u);
    CHECK(0b1'1011'1110'1110u == data[0]);
}

TEST_CASE("across multiple storage elements", "[field insert]") {
    using F = field<"", std::uint32_t>::located<at{0_dw, 17_msb, 2_lsb}>;
    std::array<std::uint8_t, 3> data{
        0b10u,
        0b0u,
        0b100u,
    };
    F::insert(data, 0b11'1010'1010'1010'11u);
    CHECK(0b1010'1110u == data[0]);
    CHECK(0b1010'1010u == data[1]);
    CHECK(0b111u == data[2]);
}

TEST_CASE("without dword index", "[field insert]") {
    using F = field<"", std::uint32_t>::located<at{17_msb, 2_lsb}>;
    std::array<std::uint8_t, 3> data{
        0b10u,
        0b0u,
        0b100u,
    };
    F::insert(data, 0b11'1010'1010'1010'11u);
    CHECK(0b1010'1110u == data[0]);
    CHECK(0b1010'1010u == data[1]);
    CHECK(0b111u == data[2]);
}

TEST_CASE("max field size", "[field insert]") {
    using F = field<"", std::uint64_t>::located<at{65_msb, 2_lsb}>;
    std::array<std::uint8_t, 9> data{
        0b10u, 0b0u, 0b0u, 0b0u, 0b0u, 0b0u, 0b0, 0b0, 0b100u,
    };
    F::insert(data, 0xeaaa'aaaa'aaaa'aaabu);
    CHECK(0b1010'1110u == data.front());
    for (auto i = std::next(std::begin(data)); i != std::prev(std::end(data));
         ++i) {
        CHECK(0b1010'1010u == *i);
    }
    CHECK(0b111u == data.back());
}

namespace {
enum struct E { Value = 0b10101 };
} // namespace

TEST_CASE("enum field type ", "[field extract]") {
    using F = field<"", E>::located<at{0_dw, 5_msb, 1_lsb}>;
    std::array<std::uint8_t, 1> data{0b1100'0001};
    //                                   ^----^
    F::insert(data, E::Value);
    CHECK(0b1110'1011 == data[0]);
}
