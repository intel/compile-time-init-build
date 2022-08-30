#include <catch2/catch_test_macros.hpp>

#include <msg/field.hpp>

namespace msg {
    using TestField1 = field<
        decltype("TestField1"_sc),
        0, 15, 0,
        std::uint32_t>;

    using TestField2 = field<
        decltype("TestField2"_sc),
        1, 23, 16,
        std::uint32_t>;

    using TestField32BitUnaligned8 = field<
        decltype("TestField32BitUnaligned8"_sc),
        0, 39, 8,
        std::uint32_t>;

    using TestField32BitUnaligned16 = field<
        decltype("TestField32BitUnaligned16"_sc),
        0, 47, 16,
        std::uint32_t>;

    using TestField32BitUnaligned24 = field<
        decltype("TestField32BitUnaligned24"_sc),
        0, 55, 24,
        std::uint32_t>;

    using TestField48BitUnaligned16 = field<
        decltype("TestField48BitUnaligned16"_sc),
        0, 63, 16,
        std::uint64_t>;

    using TestField64BitAligned = field<
        decltype("TestField64BitAligned"_sc),
        0, 63, 0,
        std::uint64_t>;

    using TestField64BitUnaligned8 = field<
        decltype("TestField64BitUnaligned8"_sc),
        0, 71, 8,
        std::uint64_t>;

    using TestField64BitUnaligned16 = field<
        decltype("TestField64BitUnaligned16"_sc),
        0, 79, 16,
        std::uint64_t>;

    using TestField64BitUnaligned24 = field<
        decltype("TestField64BitUnaligned24"_sc),
        0, 87, 24,
        std::uint64_t>;


    TEST_CASE("TestFieldExtract", "[field]") {
        std::array<std::uint32_t, 2> data{0xCAFEF00D, 0x90D5F00D};

        REQUIRE(0xF00D == TestField1::extract(data));
        REQUIRE(0xD5 == TestField2::extract(data));
    }

    TEST_CASE("TestFieldExtract32BitUnaligned8", "[field]") {
        std::array<std::uint32_t, 2> data{0x04030201, 0x08070605};

        REQUIRE(0x05040302 == TestField32BitUnaligned8::extract(data));
    }

    TEST_CASE("TestFieldExtract32BitUnaligned16", "[field]") {
        std::array<std::uint32_t, 2> data{0xF00D0000, 0x570cc001};

        REQUIRE(0xC001F00D == TestField32BitUnaligned16::extract(data));
    }

    TEST_CASE("TestFieldExtract32BitUnaligned24", "[field]") {
        std::array<std::uint32_t, 2> data{0x04030201, 0x08070605};

        REQUIRE(0x07060504 == TestField32BitUnaligned24::extract(data));
    }

    TEST_CASE("TestFieldExtract48BitUnaligned16", "[field]") {
        std::array<std::uint32_t, 2> data{0x04030201, 0x08070605};

        REQUIRE(0x080706050403UL == TestField48BitUnaligned16::extract(data));
    }

    TEST_CASE("TestFieldExtract64BitAligned", "[field]") {
        std::array<std::uint32_t, 2> data{0x04030201, 0x08070605};

        REQUIRE(0x0807060504030201UL == TestField64BitAligned::extract(data));
    }

    TEST_CASE("TestFieldExtract64BitUnaligned8", "[field]") {
        std::array<std::uint32_t, 3> data{0x44332211, 0x88776655, 0xCCBBAA99};

        REQUIRE(0x9988776655443322UL == TestField64BitUnaligned8::extract(data));
    }

    TEST_CASE("TestFieldExtract64BitUnaligned16", "[field]") {
        std::array<std::uint32_t, 3> data{0x44332211, 0x88776655, 0xCCBBAA99};

        REQUIRE(0xAA99887766554433UL == TestField64BitUnaligned16::extract(data));
    }

    TEST_CASE("TestFieldExtract64BitUnaligned24", "[field]") {
        std::array<std::uint32_t, 3> data{0x44332211, 0x88776655, 0xCCBBAA99};

        REQUIRE(0xBBAA998877665544UL == TestField64BitUnaligned24::extract(data));
    }




    TEST_CASE("TestFieldInsert32BitUnaligned8", "[field]") {
        std::array<std::uint32_t, 2> data{0, 0};

        TestField32BitUnaligned8 field{0x55443322};
        field.insert(data);

        REQUIRE(0x44332200 == data[0]);
        REQUIRE(0x00000055 == data[1]);
    }

    TEST_CASE("TestFieldInsert32BitUnaligned16", "[field]") {
        std::array<std::uint32_t, 2> data{0, 0};

        TestField32BitUnaligned16 field{0x66554433};
        field.insert(data);

        REQUIRE(0x44330000 == data[0]);
        REQUIRE(0x00006655 == data[1]);
    }

    TEST_CASE("TestFieldInsert32BitUnaligned24", "[field]") {
        std::array<std::uint32_t, 2> data{0, 0};

        TestField32BitUnaligned24 field{0x77665544};
        field.insert(data);

        REQUIRE(0x44000000 == data[0]);
        REQUIRE(0x00776655 == data[1]);
    }

    TEST_CASE("TestFieldInsert48BitUnaligned16", "[field]") {
        std::array<std::uint32_t, 2> data{0, 0};

        TestField48BitUnaligned16 field{0x665544332211ul};
        field.insert(data);

        REQUIRE(0x22110000 == data[0]);
        REQUIRE(0x66554433 == data[1]);
    }

    TEST_CASE("TestFieldInsert64BitAligned", "[field]") {
        std::array<std::uint32_t, 2> data{0, 0};

        TestField64BitAligned field{0x8877665544332211ul};
        field.insert(data);

        REQUIRE(0x44332211 == data[0]);
        REQUIRE(0x88776655 == data[1]);
    }

    TEST_CASE("TestFieldInsert64BitUnaligned8", "[field]") {
        std::array<std::uint32_t, 3> data{0, 0, 0};

        TestField64BitUnaligned8 field{0x8877665544332211ul};
        field.insert(data);

        REQUIRE(0x33221100 == data[0]);
        REQUIRE(0x77665544 == data[1]);
        REQUIRE(0x00000088 == data[2]);
    }

    TEST_CASE("TestFieldInsert64BitUnaligned16", "[field]") {
        std::array<std::uint32_t, 3> data{0, 0, 0};

        TestField64BitUnaligned16 field{0x8877665544332211ul};
        field.insert(data);

        REQUIRE(0x22110000 == data[0]);
        REQUIRE(0x66554433 == data[1]);
        REQUIRE(0x00008877 == data[2]);
    }

    TEST_CASE("TestFieldInsert64BitUnaligned24", "[field]") {
        std::array<std::uint32_t, 3> data{0, 0, 0};

        TestField64BitUnaligned24 field{0x8877665544332211ul};
        field.insert(data);

        REQUIRE(0x11000000 == data[0]);
        REQUIRE(0x55443322 == data[1]);
        REQUIRE(0x00887766 == data[2]);
    }

    TEST_CASE("TestFieldInsert", "[field]") {
        std::array<std::uint32_t, 2> data{0xC001F00D, 0x5000c001};

        TestField1 f1{0xbead};
        TestField2 f2{0xd0};

        f1.insert(data);
        f2.insert(data);

        REQUIRE(0xC001BEAD == data[0]);
        REQUIRE(0x50d0c001 == data[1]);
    }
}
