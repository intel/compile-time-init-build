#include "gtest/gtest.h"

#include <container/ClumpyIntegralMap.hpp>

//TEST(ClumpTest, EmptyConstruction) {
//    constexpr auto map =
//        makeClumpyMap<int, int>();
//}

TEST(ClumpTest, SingleConstruction) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10)
        );
}

TEST(ClumpTest, SingleConstruction_WithValidAccess) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10)
        );

    auto const result = map[5];

    ASSERT_TRUE(result);
    ASSERT_EQ(10, *result);
}

TEST(ClumpTest, SingleConstruction_LowerOutOfBoundAccess) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10)
        );

    auto const result = map[4];
    ASSERT_FALSE(result);
}

TEST(ClumpTest, SingleConstruction_UpperOutOfBoundAccess_1) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10)
        );

    auto const result = map[6];
    ASSERT_FALSE(result);
}

TEST(ClumpTest, SingleConstruction_UpperOutOfBoundAccess_2) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10)
        );

    auto const result = map[7];
    ASSERT_FALSE(result);
}

TEST(ClumpTest, SingleConstruction_GapInvalidAccess_1) {
     constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10),
            clumpyPair<7>(20)
        );

    auto const result = map[6];
    ASSERT_FALSE(result);
}

TEST(ClumpTest, SingleConstruction_GapInvalidAccess_2) {
     constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10),
            clumpyPair<8>(20)
        );

    auto const result = map[6];
    ASSERT_FALSE(result);
}

TEST(ClumpTest, SingleConstruction_GapInvalidAccess_3) {
     constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10),
            clumpyPair<8>(20)
        );

    auto const result = map[7];
    ASSERT_FALSE(result);
}

TEST(ClumpTest, SingleConstruction_GapInvalidAccess_4) {
     constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10),
            clumpyPair<9>(20)
        );

    auto const result = map[7];
    ASSERT_FALSE(result);
}


TEST(ClumpTest, TwoEntryClump) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10),
            clumpyPair<6>(13)
        );

    ASSERT_FALSE(map[3]);
    ASSERT_FALSE(map[4]);
    ASSERT_EQ(10, *map[5]);
    ASSERT_EQ(13, *map[6]);
    ASSERT_FALSE(map[7]);
    ASSERT_FALSE(map[8]);
}


TEST(ClumpTest, ThreeEntryClump) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10),
            clumpyPair<6>(13),
            clumpyPair<7>(16)
        );

    ASSERT_FALSE(map[3]);
    ASSERT_FALSE(map[4]);
    ASSERT_EQ(10, *map[5]);
    ASSERT_EQ(13, *map[6]);
    ASSERT_EQ(16, *map[7]);
    ASSERT_FALSE(map[8]);
    ASSERT_FALSE(map[9]);
}


TEST(ClumpTest, FourEntryClump) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<4>(8),
            clumpyPair<5>(10),
            clumpyPair<6>(13),
            clumpyPair<7>(16)
        );

    ASSERT_FALSE(map[2]);
    ASSERT_FALSE(map[3]);
    ASSERT_EQ(8, *map[4]);
    ASSERT_EQ(10, *map[5]);
    ASSERT_EQ(13, *map[6]);
    ASSERT_EQ(16, *map[7]);
    ASSERT_FALSE(map[8]);
    ASSERT_FALSE(map[9]);
}


TEST(ClumpTest, TwoTwoEntryClumps_1) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10),
            clumpyPair<6>(13),
            clumpyPair<8>(34),
            clumpyPair<9>(36)
        );

    ASSERT_FALSE(map[3]);
    ASSERT_FALSE(map[4]);
    ASSERT_EQ(10, *map[5]);
    ASSERT_EQ(13, *map[6]);
    ASSERT_FALSE(map[7]);
    ASSERT_EQ(34, *map[8]);
    ASSERT_EQ(36, *map[9]);
    ASSERT_FALSE(map[10]);
    ASSERT_FALSE(map[11]);
}


TEST(ClumpTest, TwoTwoEntryClumps_2) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10),
            clumpyPair<6>(13),
            clumpyPair<9>(34),
            clumpyPair<10>(36)
        );

    ASSERT_FALSE(map[3]);
    ASSERT_FALSE(map[4]);
    ASSERT_EQ(10, *map[5]);
    ASSERT_EQ(13, *map[6]);
    ASSERT_FALSE(map[7]);
    ASSERT_FALSE(map[8]);
    ASSERT_EQ(34, *map[9]);
    ASSERT_EQ(36, *map[10]);
    ASSERT_FALSE(map[11]);
    ASSERT_FALSE(map[12]);
}


TEST(ClumpTest, TwoTwoEntryClumps_3) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10),
            clumpyPair<6>(13),
            clumpyPair<10>(34),
            clumpyPair<11>(36)
        );

    ASSERT_FALSE(map[3]);
    ASSERT_FALSE(map[4]);
    ASSERT_EQ(10, *map[5]);
    ASSERT_EQ(13, *map[6]);
    ASSERT_FALSE(map[7]);
    ASSERT_FALSE(map[8]);
    ASSERT_FALSE(map[9]);
    ASSERT_EQ(34, *map[10]);
    ASSERT_EQ(36, *map[11]);
    ASSERT_FALSE(map[12]);
    ASSERT_FALSE(map[13]);
}


TEST(ClumpTest, FourEntryClump_OutOfOrderKeys) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<4>(8),
            clumpyPair<7>(16),
            clumpyPair<6>(13),
            clumpyPair<5>(10)
        );

    ASSERT_FALSE(map[2]);
    ASSERT_FALSE(map[3]);
    ASSERT_EQ(8, *map[4]);
    ASSERT_EQ(10, *map[5]);
    ASSERT_EQ(13, *map[6]);
    ASSERT_EQ(16, *map[7]);
    ASSERT_FALSE(map[8]);
    ASSERT_FALSE(map[9]);
}


TEST(ClumpTest, ThreeClumps) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10),
            clumpyPair<7>(13),
            clumpyPair<12>(42)
        );

    ASSERT_FALSE(map[3]);
    ASSERT_FALSE(map[4]);
    ASSERT_EQ(10, *map[5]);
    ASSERT_FALSE(map[6]);
    ASSERT_EQ(13, *map[7]);
    ASSERT_FALSE(map[8]);
    ASSERT_FALSE(map[9]);

    ASSERT_FALSE(map[10]);
    ASSERT_FALSE(map[11]);
    ASSERT_EQ(42, *map[12]);
    ASSERT_FALSE(map[13]);
    ASSERT_FALSE(map[14]);
}


TEST(ClumpTest, FourClumps) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<5>(10),
            clumpyPair<15>(13),
            clumpyPair<17>(42),
            clumpyPair<21>(56)
        );

    ASSERT_FALSE(map[3]);
    ASSERT_FALSE(map[4]);
    ASSERT_EQ(10, *map[5]);
    ASSERT_FALSE(map[6]);
    ASSERT_FALSE(map[7]);

    ASSERT_FALSE(map[13]);
    ASSERT_FALSE(map[14]);
    ASSERT_EQ(13, *map[15]);
    ASSERT_FALSE(map[16]);
    ASSERT_EQ(42, *map[17]);
    ASSERT_FALSE(map[18]);
    ASSERT_FALSE(map[19]);
    ASSERT_FALSE(map[20]);
    ASSERT_EQ(56, *map[21]);
    ASSERT_FALSE(map[22]);
    ASSERT_FALSE(map[23]);
}


TEST(ClumpTest, FourClumps_OutOfOrder) {
    constexpr auto map =
        makeClumpyMap<int, int>(
            clumpyPair<21>(56),
            clumpyPair<5>(10),
            clumpyPair<17>(42),
            clumpyPair<15>(13)
        );

    ASSERT_FALSE(map[3]);
    ASSERT_FALSE(map[4]);
    ASSERT_EQ(10, *map[5]);
    ASSERT_FALSE(map[6]);
    ASSERT_FALSE(map[7]);

    ASSERT_FALSE(map[13]);
    ASSERT_FALSE(map[14]);
    ASSERT_EQ(13, *map[15]);
    ASSERT_FALSE(map[16]);
    ASSERT_EQ(42, *map[17]);
    ASSERT_FALSE(map[18]);
    ASSERT_FALSE(map[19]);
    ASSERT_FALSE(map[20]);
    ASSERT_EQ(56, *map[21]);
    ASSERT_FALSE(map[22]);
    ASSERT_FALSE(map[23]);
}