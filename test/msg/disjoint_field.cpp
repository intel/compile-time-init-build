#include <catch2/catch_test_macros.hpp>

#include <msg/Field.hpp>
#include <msg/DisjointField.hpp>

#include <cib/tuple.hpp>

#include <array>
#include <cstdint>


namespace {;

    using SubField1 = Field<
        decltype("SubField1"_sc),
        0, 15, 0,
        std::uint32_t>;

    using SingularTestField = DisjointField<
        decltype("TestField"_sc),
        cib::tuple<SubField1>>;

    TEST_CASE("SingularDisjointFieldExtract", "[disjoint_field]") {
        std::array<std::uint32_t, 1> data{0xCAFEF00D};
        REQUIRE(0xF00D == SingularTestField::extract(data));
    }

    TEST_CASE("SingularDisjointFieldInsert", "[disjoint_field]") {
        std::array<std::uint32_t, 1> data{};
        SingularTestField f{0xc001};
        f.insert(data);
        REQUIRE(0xc001 == data[0]);
    }


    using UpperSubField = Field<
        decltype("UpperSubField"_sc),
        0, 7, 0,
        std::uint32_t>;

    using LowerSubField = Field<
        decltype("LowerSubField"_sc),
        1, 7, 0,
        std::uint32_t>;

    using DualTestField = DisjointField<
        decltype("DualTestField"_sc),
        cib::tuple<UpperSubField, LowerSubField>>;

    TEST_CASE("DualDisjointFieldExtract", "[disjoint_field]") {
        std::array<std::uint32_t, 2> data{0x00000012, 0x00000034};
        REQUIRE(0x1234 == DualTestField::extract(data));
    }

    TEST_CASE("DualDisjointFieldInsert", "[disjoint_field]") {
        std::array<std::uint32_t, 2> data{};
        DualTestField f{0xabcd};
        f.insert(data);
        REQUIRE(0xab == data[0]);
        REQUIRE(0xcd == data[1]);
    }
}
