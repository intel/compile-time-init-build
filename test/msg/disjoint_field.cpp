#include <msg/disjoint_field.hpp>
#include <msg/field.hpp>

#include <stdx/tuple.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>

namespace msg {
using SubField1 = field<decltype("SubField1"_sc), 0, 15, 0, std::uint32_t>;

using SingularTestField =
    disjoint_field<decltype("TestField"_sc), stdx::tuple<SubField1>>;

TEST_CASE("Singulardisjoint_fieldExtract", "[disjoint_field]") {
    std::array<std::uint32_t, 1> data{0xCAFEF00D};
    REQUIRE(0xF00D == SingularTestField::extract(data));
}

TEST_CASE("Singulardisjoint_fieldInsert", "[disjoint_field]") {
    std::array<std::uint32_t, 1> data{};
    SingularTestField f{0xc001};
    f.insert(data);
    REQUIRE(0xc001 == data[0]);
}

using UpperSubField =
    field<decltype("UpperSubField"_sc), 0, 7, 0, std::uint32_t>;

using LowerSubField =
    field<decltype("LowerSubField"_sc), 1, 7, 0, std::uint32_t>;

using DualTestField = disjoint_field<decltype("DualTestField"_sc),
                                     stdx::tuple<UpperSubField, LowerSubField>>;

TEST_CASE("Dualdisjoint_fieldExtract", "[disjoint_field]") {
    std::array<std::uint32_t, 2> data{0x00000012, 0x00000034};
    REQUIRE(0x1234 == DualTestField::extract(data));
}

TEST_CASE("Dualdisjoint_fieldInsert", "[disjoint_field]") {
    std::array<std::uint32_t, 2> data{};
    DualTestField f{0xabcd};
    f.insert(data);
    REQUIRE(0xab == data[0]);
    REQUIRE(0xcd == data[1]);
}
} // namespace msg
