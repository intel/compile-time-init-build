#include <msg/detail/rle_codec.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include <array>
#include <cstddef>

using Catch::Matchers::RangeEquals;

namespace {
auto operator"" _b(unsigned long long int v) -> std::byte {
    return static_cast<std::byte>(v);
}
} // namespace

namespace msg::detail {

// Encoding starts with number of zeros (0-255)
// followed by number of ones (0-255)
// and repeats.

TEST_CASE("rle_codec can encode all zeros", "[rle_codec]") {
    using bs = stdx::bitset<10>;
    using codec = rle_codec<bs>;

    CHECK_THAT(codec::encode(bs{}),
               RangeEquals(std::to_array<std::byte>({10_b})));
}

TEST_CASE("rle_codec can encode all ones", "[rle_codec]") {
    using bs = stdx::bitset<12>;
    using codec = rle_codec<bs>;

    CHECK_THAT(codec::encode(~bs{}),
               RangeEquals(std::to_array<std::byte>({0_b, 12_b})));
}

TEST_CASE("rle_codec can encode all zeros for large bit count", "[rle_codec]") {
    using bs = stdx::bitset<512>;
    using codec = rle_codec<bs>;

    CHECK_THAT(codec::encode(bs{}), RangeEquals(std::to_array<std::byte>(
                                        {255_b, 0_b, 255_b, 0_b, 2_b})));
}

TEST_CASE("rle_codec can encode all ones for large bit count", "[rle_codec]") {
    using bs = stdx::bitset<512>;
    using codec = rle_codec<bs>;

    CHECK_THAT(codec::encode(~bs{}), RangeEquals(std::to_array<std::byte>(
                                         {0_b, 255_b, 0_b, 255_b, 0_b, 2_b})));
}

TEST_CASE("rle_codec can encode alternating bits", "[rle_codec]") {
    using bs = stdx::bitset<8>;
    using codec = rle_codec<bs>;

    CHECK_THAT(codec::encode(bs{stdx::place_bits, 0, 2, 4, 6}),
               RangeEquals(std::to_array<std::byte>(
                   {0_b, 1_b, 1_b, 1_b, 1_b, 1_b, 1_b, 1_b, 1_b})));

    CHECK_THAT(codec::encode(bs{stdx::place_bits, 1, 3, 5, 7}),
               RangeEquals(std::to_array<std::byte>(
                   {1_b, 1_b, 1_b, 1_b, 1_b, 1_b, 1_b, 1_b})));
}

TEST_CASE("rle_codec can decode", "[rle_codec]") {
    using bs = stdx::bitset<500>;
    using codec = rle_codec<bs>;
    auto const b1 = bs{stdx::place_bits, 1, 2, 3, 499};
    auto const b2 = bs{stdx::place_bits, 2, 3, 5, 7, 11, 13, 17, 400};
    auto const bzero = bs{};
    auto const bone = ~bs{};

    CHECK(codec::decode(codec::encode(b1).cbegin()) == b1);
    CHECK(codec::decode(codec::encode(b2).cbegin()) == b2);
    CHECK(codec::decode(codec::encode(bzero).cbegin()) == bzero);
    CHECK(codec::decode(codec::encode(bone).cbegin()) == bone);
}

} // namespace msg::detail
