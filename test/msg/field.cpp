#include <msg/field.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using namespace msg;

using TestField1 =
    field<"TestField1", std::uint32_t>::located<at{0_dw, 15_msb, 0_lsb}>;

using TestField2 =
    field<"TestField2", std::uint32_t>::located<at{1_dw, 23_msb, 16_lsb}>;

using TestField32BitUnaligned8 =
    field<"TestField32BitUnaligned8",
          std::uint32_t>::located<at{0_dw, 39_msb, 8_lsb}>;

using TestField32BitUnaligned16 =
    field<"TestField32BitUnaligned16",
          std::uint32_t>::located<at{0_dw, 47_msb, 16_lsb}>;

using TestField32BitUnaligned24 =
    field<"TestField32BitUnaligned24",
          std::uint32_t>::located<at{0_dw, 55_msb, 24_lsb}>;

using TestField48BitUnaligned16 =
    field<"TestField48BitUnaligned16",
          std::uint64_t>::located<at{0_dw, 63_msb, 16_lsb}>;

using TestField64BitAligned =
    field<"TestField64BitAligned",
          std::uint64_t>::located<at{0_dw, 63_msb, 0_lsb}>;

using TestField64BitUnaligned8 =
    field<"TestField64BitUnaligned8",
          std::uint64_t>::located<at{0_dw, 71_msb, 8_lsb}>;

using TestField64BitUnaligned16 =
    field<"TestField64BitUnaligned16",
          std::uint64_t>::located<at{0_dw, 79_msb, 16_lsb}>;

using TestField64BitUnaligned24 =
    field<"TestField64BitUnaligned24",
          std::uint64_t>::located<at{0_dw, 87_msb, 24_lsb}>;

using DualTestField =
    field<"DualTestField", std::uint32_t>::located<at{0_dw, 7_msb, 0_lsb},
                                                   at{1_dw, 7_msb, 0_lsb}>;
} // namespace

TEST_CASE("TestFieldExtract", "[field]") {
    std::array<std::uint32_t, 2> data{0xCAFEF00D, 0x90D5F00D};
    CHECK(0xF00D == TestField1::extract(data));
    CHECK(0xD5 == TestField2::extract(data));
}

TEST_CASE("TestFieldExtract32BitUnaligned8", "[field]") {
    std::array<std::uint32_t, 2> data{0x04030201, 0x08070605};
    CHECK(0x05040302 == TestField32BitUnaligned8::extract(data));
}

TEST_CASE("TestFieldExtract32BitUnaligned16", "[field]") {
    std::array<std::uint32_t, 2> data{0xF00D0000, 0x570cc001};
    CHECK(0xC001F00D == TestField32BitUnaligned16::extract(data));
}

TEST_CASE("TestFieldExtract32BitUnaligned24", "[field]") {
    std::array<std::uint32_t, 2> data{0x04030201, 0x08070605};
    CHECK(0x07060504 == TestField32BitUnaligned24::extract(data));
}

TEST_CASE("TestFieldExtract48BitUnaligned16", "[field]") {
    std::array<std::uint32_t, 2> data{0x04030201, 0x08070605};
    CHECK(0x080706050403UL == TestField48BitUnaligned16::extract(data));
}

TEST_CASE("TestFieldExtract64BitAligned", "[field]") {
    std::array<std::uint32_t, 2> data{0x04030201, 0x08070605};
    CHECK(0x0807060504030201UL == TestField64BitAligned::extract(data));
}

TEST_CASE("TestFieldExtract64BitUnaligned8", "[field]") {
    std::array<std::uint32_t, 3> data{0x44332211, 0x88776655, 0xCCBBAA99};
    CHECK(0x9988776655443322UL == TestField64BitUnaligned8::extract(data));
}

TEST_CASE("TestFieldExtract64BitUnaligned16", "[field]") {
    std::array<std::uint32_t, 3> data{0x44332211, 0x88776655, 0xCCBBAA99};
    CHECK(0xAA99887766554433UL == TestField64BitUnaligned16::extract(data));
}

TEST_CASE("TestFieldExtract64BitUnaligned24", "[field]") {
    std::array<std::uint32_t, 3> data{0x44332211, 0x88776655, 0xCCBBAA99};
    CHECK(0xBBAA998877665544UL == TestField64BitUnaligned24::extract(data));
}

TEST_CASE("TestFieldInsert32BitUnaligned8", "[field]") {
    std::array<std::uint32_t, 2> data{0, 0};

    TestField32BitUnaligned8 field{};
    field.insert(data, 0x55443322u);

    CHECK(0x44332200 == data[0]);
    CHECK(0x00000055 == data[1]);
}

TEST_CASE("TestFieldInsert32BitUnaligned16", "[field]") {
    std::array<std::uint32_t, 2> data{0, 0};

    TestField32BitUnaligned16 field{};
    field.insert(data, 0x66554433u);

    CHECK(0x44330000 == data[0]);
    CHECK(0x00006655 == data[1]);
}

TEST_CASE("TestFieldInsert32BitUnaligned24", "[field]") {
    std::array<std::uint32_t, 2> data{0, 0};

    TestField32BitUnaligned24 field{};
    field.insert(data, 0x77665544u);

    CHECK(0x44000000 == data[0]);
    CHECK(0x00776655 == data[1]);
}

TEST_CASE("TestFieldInsert48BitUnaligned16", "[field]") {
    std::array<std::uint32_t, 2> data{0, 0};

    TestField48BitUnaligned16 field{};
    field.insert(data, 0x665544332211ul);

    CHECK(0x22110000 == data[0]);
    CHECK(0x66554433 == data[1]);
}

TEST_CASE("TestFieldInsert64BitAligned", "[field]") {
    std::array<std::uint32_t, 2> data{0, 0};

    TestField64BitAligned field{};
    field.insert(data, 0x8877665544332211ul);

    CHECK(0x44332211 == data[0]);
    CHECK(0x88776655 == data[1]);
}

TEST_CASE("TestFieldInsert64BitUnaligned8", "[field]") {
    std::array<std::uint32_t, 3> data{0, 0, 0};

    TestField64BitUnaligned8 field{};
    field.insert(data, 0x8877665544332211ul);

    CHECK(0x33221100 == data[0]);
    CHECK(0x77665544 == data[1]);
    CHECK(0x00000088 == data[2]);
}

TEST_CASE("TestFieldInsert64BitUnaligned16", "[field]") {
    std::array<std::uint32_t, 3> data{0, 0, 0};

    TestField64BitUnaligned16 field{};
    field.insert(data, 0x8877665544332211ul);

    CHECK(0x22110000 == data[0]);
    CHECK(0x66554433 == data[1]);
    CHECK(0x00008877 == data[2]);
}

TEST_CASE("TestFieldInsert64BitUnaligned24", "[field]") {
    std::array<std::uint32_t, 3> data{0, 0, 0};

    TestField64BitUnaligned24 field{};
    field.insert(data, 0x8877665544332211ul);

    CHECK(0x11000000 == data[0]);
    CHECK(0x55443322 == data[1]);
    CHECK(0x00887766 == data[2]);
}

TEST_CASE("TestFieldInsert", "[field]") {
    std::array<std::uint32_t, 2> data{0xC001F00D, 0x5000c001};

    TestField1 f1{};
    TestField2 f2{};

    f1.insert(data, 0xbead);
    f2.insert(data, 0xd0);

    CHECK(0xC001BEAD == data[0]);
    CHECK(0x50d0c001 == data[1]);
}

TEST_CASE("Dualdisjoint_fieldExtract", "[field]") {
    std::array<std::uint32_t, 2> data{0x00000012, 0x00000034};
    CHECK(0x1234 == DualTestField::extract(data));
}

TEST_CASE("Dualdisjoint_fieldInsert", "[field]") {
    std::array<std::uint32_t, 2> data{};
    DualTestField f{};
    f.insert(data, 0xabcd);
    CHECK(0xab == data[0]);
    CHECK(0xcd == data[1]);
}
