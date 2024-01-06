#include <msg/detail/rle_codec.hpp>

#include <stdx/compiler.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include <array>
#include <cstddef>

using Catch::Matchers::RangeEquals;

namespace {
CONSTEVAL auto operator"" _b(unsigned long long v) -> std::byte {
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

    CHECK_THAT(codec::encode(bs{}), RangeEquals(std::array{10_b}));
}

TEST_CASE("rle_codec can encode all ones", "[rle_codec]") {
    using bs = stdx::bitset<12>;
    using codec = rle_codec<bs>;

    CHECK_THAT(codec::encode(~bs{}), RangeEquals(std::array{0_b, 12_b}));
}

TEST_CASE("rle_codec can encode all zeros for large bit count", "[rle_codec]") {
    using bs = stdx::bitset<512>;
    using codec = rle_codec<bs>;

    CHECK_THAT(codec::encode(bs{}),
               RangeEquals(std::array{255_b, 0_b, 255_b, 0_b, 2_b}));
}

TEST_CASE("rle_codec can encode all ones for large bit count", "[rle_codec]") {
    using bs = stdx::bitset<512>;
    using codec = rle_codec<bs>;

    CHECK_THAT(codec::encode(~bs{}),
               RangeEquals(std::array{0_b, 255_b, 0_b, 255_b, 0_b, 2_b}));
}

TEST_CASE("rle_codec can encode alternating bits", "[rle_codec]") {
    using bs = stdx::bitset<8>;
    using codec = rle_codec<bs>;

    CHECK_THAT(
        codec::encode(bs{stdx::place_bits, 0, 2, 4, 6}),
        RangeEquals(std::array{0_b, 1_b, 1_b, 1_b, 1_b, 1_b, 1_b, 1_b, 1_b}));

    CHECK_THAT(codec::encode(bs{stdx::place_bits, 1, 3, 5, 7}),
               RangeEquals(std::array{1_b, 1_b, 1_b, 1_b, 1_b, 1_b, 1_b, 1_b}));
}

TEST_CASE("rle_decoder can iterate chunks 1", "[rle_codec]") {
    using bs = stdx::bitset<8>;
    using decoder = rle_decoder<bs>;

    auto rle = std::array{1_b, 1_b, 1_b, 1_b, 1_b, 1_b, 1_b, 1_b};

    decoder dec{rle.data()};
    auto chunk = dec.make_chunk_enumerator();
    std::size_t bit{0};

    REQUIRE(chunk.bit_chunk() == -1);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == 1);
    REQUIRE(bit == 1);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == -1);
    REQUIRE(bit == 2);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == 1);
    REQUIRE(bit == 3);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == -1);
    REQUIRE(bit == 4);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == 1);
    REQUIRE(bit == 5);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == -1);
    REQUIRE(bit == 6);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == 1);
    REQUIRE(bit == 7);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == 0);
    REQUIRE(bit == 8);
}

TEST_CASE("rle_decoder can iterate chunks 2", "[rle_codec]") {
    using bs = stdx::bitset<8>;
    using decoder = rle_decoder<bs>;

    auto rle = std::array{0_b, 2_b, 2_b, 2_b, 0_b, 2_b};
    decoder dec{rle.data()};
    auto chunk = dec.make_chunk_enumerator();
    std::size_t bit{0};

    REQUIRE(chunk.bit_chunk() == 2);

    bit = chunk.advance(2, bit);
    REQUIRE(chunk.bit_chunk() == -2);
    REQUIRE(bit == 2);

    bit = chunk.advance(2, bit);
    REQUIRE(chunk.bit_chunk() == 2);
    REQUIRE(bit == 4);

    bit = chunk.advance(2, bit);
    REQUIRE(chunk.bit_chunk() == 2);
    REQUIRE(bit == 6);

    bit = chunk.advance(2, bit);
    REQUIRE(chunk.bit_chunk() == 0);
    REQUIRE(bit == 8);
}

TEST_CASE("rle_decoder can iterate chunks 3", "[rle_codec]") {
    using bs = stdx::bitset<1000>;
    using decoder = rle_decoder<bs>;

    auto rle = std::array{255_b, 0_b, 255_b, 0_b, 255_b, 235_b};
    decoder dec{rle.data()};
    auto chunk = dec.make_chunk_enumerator();

    std::size_t bit{0};

    REQUIRE(chunk.bit_chunk() == -255);

    bit = chunk.advance(510, bit);
    REQUIRE(chunk.bit_chunk() == -255);
    REQUIRE(bit == 510);

    bit = chunk.advance(255, bit);
    REQUIRE(chunk.bit_chunk() == 235);
    REQUIRE(bit == 765);

    bit = chunk.advance(235, bit);
    REQUIRE(chunk.bit_chunk() == 0);
    REQUIRE(bit == 1000);
}

TEST_CASE("rle_decoder can iterate sub-chunks", "[rle_codec]") {
    using bs = stdx::bitset<8>;
    using decoder = rle_decoder<bs>;

    auto rle = std::array{0_b, 2_b, 2_b, 2_b, 0_b, 2_b};
    decoder dec{rle.data()};
    auto chunk = dec.make_chunk_enumerator();
    std::size_t bit{0};

    REQUIRE(chunk.bit_chunk() == 2);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == 1);
    REQUIRE(bit == 1);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == -2);
    REQUIRE(bit == 2);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == -1);
    REQUIRE(bit == 3);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == 2);
    REQUIRE(bit == 4);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == 1);
    REQUIRE(bit == 5);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == 2);
    REQUIRE(bit == 6);

    bit = chunk.advance(1, bit);
    REQUIRE(chunk.bit_chunk() == 1);
    REQUIRE(bit == 7);

    bit = chunk.advance(1, bit);
    // we are now out of bits and should get zero back
    // and not attempt to read beyond end of rle data
    REQUIRE(chunk.bit_chunk() == 0);
    REQUIRE(bit == 8);

    bit = chunk.advance(10, bit);
    REQUIRE(chunk.bit_chunk() == 0);
    REQUIRE(bit == 8);
}

TEST_CASE("rle_decoder stops after num_bits", "[rle_codec]") {
    using bs = stdx::bitset<8>;
    using decoder = rle_decoder<bs>;

    // contains extra data which should be ignored
    auto rle = std::array{0_b, 2_b, 2_b, 2_b, 0_b, 2_b, 255_b, 255_b};
    decoder dec{rle.data()};
    auto chunk = dec.make_chunk_enumerator();
    std::size_t bit{0};

    REQUIRE(chunk.bit_chunk() == 2);
    bit = chunk.advance(100, bit);
    REQUIRE(chunk.bit_chunk() == 0);
    REQUIRE(bit == 8);
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

TEST_CASE("rle_codec can decode multiple zero pads", "[rle_codec]") {
    using bs = stdx::bitset<2000>;
    using codec = rle_codec<bs>;
    auto const b1 = bs{stdx::place_bits, 1, 2, 3, 499, 1999};
    auto const b2 = bs{stdx::place_bits, 2, 3, 5, 7, 11, 13, 17, 400, 1999};
    auto const bzero = bs{};
    auto const bone = ~bs{};

    CHECK(codec::decode(codec::encode(b1).cbegin()) == b1);
    CHECK(codec::decode(codec::encode(b2).cbegin()) == b2);
    CHECK(codec::decode(codec::encode(bzero).cbegin()) == bzero);
    CHECK(codec::decode(codec::encode(bone).cbegin()) == bone);
}

TEST_CASE("rle_intersect works with 1 rle bitset", "[rle_intersect]") {
    using bs = stdx::bitset<8>;
    using codec = rle_codec<bs>;
    using decoder_t = rle_decoder<bs>;

    auto rle_1 = codec::encode(bs{"00011100"});

    auto dec_1 = decoder_t{rle_1.cbegin()};

    auto intersection =
        rle_intersect{std::forward<decoder_t>(dec_1)}.get_bitset();

    CHECK(intersection == bs{"00011100"});
}

TEST_CASE("rle_intersect works with 2 rle bitsets", "[rle_intersect]") {
    using bs = stdx::bitset<8>;
    using codec = rle_codec<bs>;
    using decoder_t = rle_decoder<bs>;

    auto rle_1 = codec::encode(bs{"00011100"});
    auto rle_2 = codec::encode(bs{"11110101"});

    auto dec_1 = decoder_t{rle_1.cbegin()};
    auto dec_2 = decoder_t{rle_2.cbegin()};

    auto intersection = rle_intersect{std::forward<decoder_t>(dec_1),
                                      std::forward<decoder_t>(dec_2)}
                            .get_bitset();

    CHECK(intersection == bs{"00010100"});
}

TEST_CASE("rle_intersect works with many rle bitsets", "[rle_intersect]") {
    using bs = stdx::bitset<32>;
    using codec = rle_codec<bs>;
    using decoder_t = rle_decoder<bs>;

    auto rle_1 = codec::encode(bs{"11111111111111110000000000000000"});
    auto rle_2 = codec::encode(bs{"11111111000000001111111100000000"});
    auto rle_3 = codec::encode(bs{"11110000111100001111000011110000"});
    auto rle_4 = codec::encode(bs{"11001100110011001100110011001100"});
    auto rle_5 = codec::encode(bs{"10101010101010101010101010101010"});
    auto expected = /* pad  */ bs{"10000000000000000000000000000000"};

    auto dec_1 = decoder_t{rle_1.cbegin()};
    auto dec_2 = decoder_t{rle_2.cbegin()};
    auto dec_3 = decoder_t{rle_3.cbegin()};
    auto dec_4 = decoder_t{rle_4.cbegin()};
    auto dec_5 = decoder_t{rle_5.cbegin()};

    auto intersection = rle_intersect{std::forward<decoder_t>(dec_1),
                                      std::forward<decoder_t>(dec_2),
                                      std::forward<decoder_t>(dec_3),
                                      std::forward<decoder_t>(dec_4),
                                      std::forward<decoder_t>(dec_5)}
                            .get_bitset();

    CHECK(intersection == expected);
}

TEST_CASE("rle_intersect for_each()", "[rle_intersect]") {
    using bs = stdx::bitset<8>;
    using codec = rle_codec<bs>;
    using decoder_t = rle_decoder<bs>;

    auto rle_1 = codec::encode(bs{"00011100"});
    auto rle_2 = codec::encode(bs{"11110101"});
    auto expected = /* pad  */ bs{"00010100"};

    auto dec_1 = decoder_t{rle_1.cbegin()};
    auto dec_2 = decoder_t{rle_2.cbegin()};

    auto intersection = rle_intersect{std::forward<decoder_t>(dec_1),
                                      std::forward<decoder_t>(dec_2)};

    bs result{};
    intersection.for_each([&](auto i) { result.set(i); });

    CHECK(result == expected);
}

} // namespace msg::detail
