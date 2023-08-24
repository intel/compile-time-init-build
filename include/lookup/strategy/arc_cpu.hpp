#pragma once

#include <lookup/direct_array_lookup.hpp>
#include <lookup/fast_hash_lookup.hpp>
#include <lookup/hash_ops.hpp>
#include <lookup/linear_search_lookup.hpp>
#include <lookup/strategies.hpp>

namespace lookup::strategy {
// benchmarked on ARC HS4X HSDK dev board
using arc_cpu = strategies<
    // 4.1 - 6.39 cycles unrolled 4x
    linear_search_lookup<3>,

    // 6.51 cycles unrolled 4x
    direct_array_lookup<25>,

    // 7.5 cycles unrolled 4x
    linear_search_lookup<4>,

    // 8.39 cycles unrolled 4x
    // 16 cycles
    /* cost = 4 */
    fast_hash_lookup<50, 0,
                     ops<subl_op<3>, xorr_op<9>, subl_op<3>>>, /* score =
                                                                  0.590423
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<2>, xorr_op<6>, addl_op<3>>>, /* score =
                                                                  0.590311
                                                                */
    fast_hash_lookup<
        50, 0, ops<addl_op<3>, xorr_op<10>, addl_op<3>>>, /* score = 0.589424 */
    fast_hash_lookup<50, 0,
                     ops<addl_op<2>, xorr_op<9>, subl_op<3>>>, /* score =
                                                                  0.588427
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<9>, subl_op<2>>>, /* score =
                                                                  0.587195
                                                                */
    fast_hash_lookup<50, 0,
                     ops<subl_op<3>, xorr_op<8>, subl_op<2>>>, /* score =
                                                                  0.587084
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<9>, subl_op<3>>>, /* score =
                                                                  0.586961
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<6>, addl_op<1>>>, /* score =
                                                                  0.585448
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<7>, subl_op<3>>>, /* score =
                                                                  0.585002
                                                                */
    fast_hash_lookup<50, 0,
                     ops<subl_op<3>, xorr_op<9>, addl_op<3>>>, /* score =
                                                                  0.584988
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<9>, addl_op<1>>>, /* score =
                                                                  0.584031
                                                                */
    fast_hash_lookup<50, 0,
                     ops<subl_op<3>, xorr_op<7>, subl_op<2>>>, /* score =
                                                                  0.583385
                                                                */
    fast_hash_lookup<50, 0,
                     ops<subl_op<3>, xorr_op<8>, addl_op<1>>>, /* score =
                                                                  0.583343
                                                                */
    fast_hash_lookup<50, 0,
                     ops<subl_op<3>, xorr_op<7>, subl_op<3>>>, /* score =
                                                                  0.581946
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<7>, addl_op<3>>>, /* score =
                                                                  0.581099
                                                                */
    fast_hash_lookup<50, 0,
                     ops<subl_op<3>, xorr_op<8>, subl_op<3>>>, /* score =
                                                                  0.579574
                                                                */
    fast_hash_lookup<50, 0,
                     ops<subl_op<3>, xorr_op<7>, addl_op<1>>>, /* score =
                                                                  0.579078
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<6>, addl_op<2>>>, /* score =
                                                                  0.578593
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<8>, subl_op<3>>>, /* score =
                                                                  0.578556
                                                                */
    fast_hash_lookup<50, 0,
                     ops<subl_op<3>, xorr_op<6>, addl_op<2>>>, /* score =
                                                                  0.577850
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<2>, xorr_op<7>, subl_op<3>>>, /* score =
                                                                  0.577618
                                                                */
    fast_hash_lookup<
        50, 0, ops<addl_op<3>, xorr_op<10>, addl_op<2>>>, /* score = 0.576333 */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<7>, subl_op<2>>>, /* score =
                                                                  0.576054
                                                                */
    fast_hash_lookup<50, 0,
                     ops<subl_op<3>, xorr_op<7>, addl_op<3>>>, /* score =
                                                                  0.575912
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<8>, subl_op<2>>>, /* score =
                                                                  0.575805
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<2>, xorr_op<8>, subl_op<3>>>, /* score =
                                                                  0.575643
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<9>, addl_op<3>>>, /* score =
                                                                  0.573534
                                                                */
    fast_hash_lookup<50, 0,
                     ops<subl_op<3>, xorr_op<9>, addl_op<2>>>, /* score =
                                                                  0.573388
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<2>, xorr_op<9>, addl_op<3>>>, /* score =
                                                                  0.572982
                                                                */
    fast_hash_lookup<50, 0,
                     ops<subl_op<3>, xorr_op<8>, addl_op<3>>>, /* score =
                                                                  0.572200
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<8>, addl_op<3>>>, /* score =
                                                                  0.570872
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<8>, addl_op<1>>>, /* score =
                                                                  0.570304
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<7>, addl_op<1>>>, /* score =
                                                                  0.570058
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<2>, xorr_op<7>, addl_op<3>>>, /* score =
                                                                  0.569539
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<2>, xorr_op<8>, addl_op<3>>>, /* score =
                                                                  0.563239
                                                                */
    fast_hash_lookup<50, 0,
                     ops<subl_op<3>, xorr_op<7>, addl_op<2>>>, /* score =
                                                                  0.559808
                                                                */
    fast_hash_lookup<50, 0,
                     ops<subl_op<3>, xorr_op<8>, addl_op<2>>>, /* score =
                                                                  0.559302
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<7>, addl_op<2>>>, /* score =
                                                                  0.556430
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<9>, addl_op<2>>>, /* score =
                                                                  0.556230
                                                                */
    fast_hash_lookup<50, 0,
                     ops<addl_op<3>, xorr_op<8>, addl_op<2>>>, /* score =
                                                                  0.548717
                                                                */

    // 9.39 cycles unrolled 4x
    linear_search_lookup<7>,

    /* cost = 3 */
    // 10.39 cycles unrolled 4x
    // 17 cycles
    fast_hash_lookup<50, 0, ops<mul_op<(854u << 1u) | 1u>>>, /* score = 0.788028
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(426u << 1u) | 1u>>>, /* score = 0.787923
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(857u << 1u) | 1u>>>, /* score = 0.787708
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(805u << 1u) | 1u>>>, /* score = 0.787554
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(813u << 1u) | 1u>>>, /* score = 0.787510
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(809u << 1u) | 1u>>>, /* score = 0.787272
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(845u << 1u) | 1u>>>, /* score = 0.787214
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(613u << 1u) | 1u>>>, /* score = 0.787119
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(683u << 1u) | 1u>>>, /* score = 0.786974
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(869u << 1u) | 1u>>>, /* score = 0.786350
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(405u << 1u) | 1u>>>, /* score = 0.785996
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(917u << 1u) | 1u>>>, /* score = 0.785899
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(810u << 1u) | 1u>>>, /* score = 0.785559
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(821u << 1u) | 1u>>>, /* score = 0.783701
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(685u << 1u) | 1u>>>, /* score = 0.782107
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(681u << 1u) | 1u>>>, /* score = 0.781311
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(677u << 1u) | 1u>>>, /* score = 0.780723
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(693u << 1u) | 1u>>>, /* score = 0.780058
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(682u << 1u) | 1u>>>, /* score = 0.779636
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(341u << 1u) | 1u>>>, /* score = 0.779557
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(661u << 1u) | 1u>>>, /* score = 0.779536
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(597u << 1u) | 1u>>>, /* score = 0.778654
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(725u << 1u) | 1u>>>, /* score = 0.778196
                                                              */
    fast_hash_lookup<50, 0, ops<mul_op<(853u << 1u) | 1u>>>, /* score = 0.775511
                                                              */

    // 10.76 cycles unrolled 4x
    // 19 cycles
    fast_hash_lookup<50, 0, ops<subl_op<3>, xorr_op<4>>>, /* score = 0.776173 */
    fast_hash_lookup<50, 0, ops<addl_op<3>, xorr_op<4>>>, /* score = 0.776945 */
    fast_hash_lookup<50, 0, ops<addl_op<3>, xorr_op<9>>>, /* score = 0.778502 */
    fast_hash_lookup<50, 0, ops<subl_op<3>, xorr_op<8>>>, /* score = 0.780110 */
    fast_hash_lookup<50, 0, ops<addl_op<2>, xorr_op<8>>>, /* score = 0.782328 */
    fast_hash_lookup<50, 0, ops<addl_op<2>, xorr_op<7>>>, /* score = 0.773400 */
    fast_hash_lookup<50, 0, ops<addl_op<2>, xorr_op<4>>>, /* score = 0.772495 */
    fast_hash_lookup<50, 0, ops<subl_op<3>, xorr_op<7>>>, /* score = 0.770501 */
    fast_hash_lookup<50, 0, ops<addl_op<3>, xorr_op<8>>>, /* score = 0.768133 */
    fast_hash_lookup<50, 0, ops<addl_op<2>, xorr_op<6>>>, /* score = 0.766878 */
    fast_hash_lookup<50, 0, ops<addl_op<2>, xorr_op<5>>>, /* score = 0.765588 */
    fast_hash_lookup<50, 0, ops<subl_op<3>, xorr_op<5>>>, /* score = 0.763783 */
    fast_hash_lookup<50, 0, ops<subl_op<3>, xorr_op<6>>>, /* score = 0.763414 */
    fast_hash_lookup<50, 0, ops<addl_op<3>, xorr_op<7>>>, /* score = 0.759413 */
    fast_hash_lookup<50, 0, ops<addl_op<3>, xorr_op<5>>>, /* score = 0.758579 */
    fast_hash_lookup<50, 0, ops<addl_op<3>, xorr_op<6>>>, /* score = 0.753757 */

    // 13 cycles unrolled 4x
    // 25 cycles
    /* cost = 9 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x5e4dbabau << 1u) | 1u>, xorr_op<13>,
                         addl_op<3>, xorr_op<6>>>, /* score = 0.016817 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x7e83c4eau << 1u) | 1u>, xorr_op<11>,
                         addl_op<3>, xorr_op<8>>>, /* score = 0.016788 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x5090cceau << 1u) | 1u>, xorr_op<16>,
                         addl_op<3>, xorr_op<7>>>, /* score = 0.016709 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x3e603515u << 1u) | 1u>, xorr_op<14>,
                         addl_op<2>, xorr_op<7>>>, /* score = 0.016666 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x0eb2cd2fu << 1u) | 1u>, xorr_op<15>,
                         addl_op<3>, xorr_op<8>>>, /* score = 0.016476 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x11da2657u << 1u) | 1u>, xorr_op<15>,
                         addl_op<3>, xorr_op<7>>>, /* score = 0.016465 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x24f7db46u << 1u) | 1u>, xorr_op<13>,
                         subl_op<3>, xorr_op<6>>>, /* score = 0.016440 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x31f9e596u << 1u) | 1u>, xorr_op<16>,
                         addl_op<3>, xorr_op<9>>>, /* score = 0.016341 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x387c94fau << 1u) | 1u>, xorr_op<5>,
                         subl_op<3>, xorr_op<15>>>, /* score = 0.016296 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x74852a8au << 1u) | 1u>, xorr_op<16>,
                         addl_op<3>, xorr_op<6>>>, /* score = 0.016133 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x3879a5d9u << 1u) | 1u>, xorr_op<16>,
                         addl_op<3>, xorr_op<8>>>, /* score = 0.015807 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x6decea95u << 1u) | 1u>, xorr_op<11>,
                         addl_op<3>, xorr_op<9>>>, /* score = 0.015437 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x6f90553du << 1u) | 1u>, xorr_op<16>,
                         addl_op<2>, xorr_op<9>>>, /* score = 0.015309 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x2586a9d6u << 1u) | 1u>, xorr_op<12>,
                         addl_op<3>, xorr_op<7>>>, /* score = 0.015263 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x3e69f531u << 1u) | 1u>, xorr_op<14>,
                         addl_op<2>, xorr_op<6>>>, /* score = 0.015048 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x30e5c295u << 1u) | 1u>, xorr_op<13>,
                         subl_op<3>, xorr_op<7>>>, /* score = 0.014988 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x56304553u << 1u) | 1u>, xorr_op<13>,
                         subl_op<3>, xorr_op<11>>>, /* score = 0.014983 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x3fd33c35u << 1u) | 1u>, xorr_op<9>,
                         addl_op<3>, xorr_op<14>>>, /* score = 0.014583 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x103d59b2u << 1u) | 1u>, xorr_op<17>,
                         subl_op<3>, xorr_op<9>>>, /* score = 0.014437 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x169df055u << 1u) | 1u>, xorr_op<12>,
                         addl_op<3>, xorr_op<8>>>, /* score = 0.014301 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x4007eaabu << 1u) | 1u>, xorr_op<15>,
                         addl_op<2>, xorr_op<6>>>, /* score = 0.013983 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x7f04ad54u << 1u) | 1u>, xorr_op<18>,
                         subl_op<3>, xorr_op<6>>>, /* score = 0.013693 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x06ecf965u << 1u) | 1u>, xorr_op<12>,
                         addl_op<3>, xorr_op<7>>>, /* score = 0.013602 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x7fadea56u << 1u) | 1u>, xorr_op<12>,
                         subl_op<3>, xorr_op<6>>>, /* score = 0.013586 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x4bae9355u << 1u) | 1u>, xorr_op<12>,
                         addl_op<3>, xorr_op<8>>>, /* score = 0.013531 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x4f7eed6bu << 1u) | 1u>, xorr_op<15>,
                         addl_op<2>, xorr_op<6>>>, /* score = 0.013336 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x32371656u << 1u) | 1u>, xorr_op<13>,
                         addl_op<3>, xorr_op<7>>>, /* score = 0.013324 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x4ed2066au << 1u) | 1u>, xorr_op<12>,
                         addl_op<2>, xorr_op<7>>>, /* score = 0.012913 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x640bbf55u << 1u) | 1u>, xorr_op<8>,
                         addl_op<3>, xorr_op<17>>>, /* score = 0.012775 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x27568029u << 1u) | 1u>, xorr_op<8>,
                         addl_op<3>, xorr_op<14>>>, /* score = 0.012483 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x1b897879u << 1u) | 1u>, xorr_op<6>,
                         addl_op<3>, xorr_op<15>>>, /* score = 0.012319 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x7d6bc0aau << 1u) | 1u>, xorr_op<8>,
                         addl_op<3>, xorr_op<11>>>, /* score = 0.012096 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x2d5f06a5u << 1u) | 1u>, xorr_op<13>,
                         addl_op<3>, xorr_op<8>>>, /* score = 0.011498 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x6bb889aau << 1u) | 1u>, xorr_op<14>,
                         subl_op<3>, xorr_op<6>>>, /* score = 0.011431 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x5e8154eau << 1u) | 1u>, xorr_op<17>,
                         subl_op<3>, xorr_op<8>>>, /* score = 0.010015 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(0x6013d32au << 1u) | 1u>, xorr_op<14>,
                         addl_op<3>, xorr_op<8>>>, /* score = 0.009885 */

    // 15.87 unrolled 4x
    // 25
    fast_hash_lookup<50, 0,
                     ops<mul_op<(689u << 1u) | 1u>, xorr_op<12>, addl_op<3>,
                         xorr_op<11>, addl_op<2>>>, /* score = 0.012407 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(169u << 1u) | 1u>, xorr_op<12>, subl_op<3>,
                         xorr_op<9>, addl_op<3>>>, /* score = 0.014352 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(938u << 1u) | 1u>, xorr_op<12>, addl_op<2>,
                         xorr_op<6>, addl_op<1>>>, /* score = 0.015374 */
    fast_hash_lookup<50, 0,
                     ops<mul_op<(813u << 1u) | 1u>, xorr_op<13>, addl_op<3>,
                         xorr_op<7>, subl_op<2>>> /* score = 0.016001 */
    >;
} // namespace lookup::strategy
