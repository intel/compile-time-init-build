#include <msg/disjoint_field.hpp>
#include <msg/message.hpp>

#include <catch2/catch_test_macros.hpp>

namespace msg {
using TestIdField = field<decltype("TestIdField"_sc), 0, 31, 24, std::uint32_t>;

using TestField1 = field<decltype("TestField1"_sc), 0, 15, 0, std::uint32_t>;

using TestField2 = field<decltype("TestField2"_sc), 1, 23, 16, std::uint32_t>;

using TestFieldLower =
    field<decltype("TestFieldLower"_sc), 1, 15, 8, std::uint32_t>;

using TestFieldUpper =
    field<decltype("TestFieldUpper"_sc), 1, 7, 0, std::uint32_t>;

using TestField3 = disjoint_field<decltype("TestField4"_sc),
                                  cib::tuple<TestFieldLower, TestFieldUpper>>;

using TestField4 = field<decltype("TestField4"_sc), 1, 31, 24, std::uint8_t>;

using TestMsg =
    message_base<decltype("TestMsg"_sc), 2, TestIdField::WithRequired<0x80>,
                 TestField1, TestField2, TestField3>;

using TestLastBitField = 
    message_base<decltype("TestMsg"_sc), 2, TestIdField::WithRequired<0x80>,
                TestField1, TestField2, TestField3, TestField4>;

TEST_CASE("TestMessageDataFieldConstruction", "[message]") {
    TestMsg msg{TestField1{0xba11}, TestField2{0x42}, TestField3{0xd00d}};

    CHECK(0xba11 == msg.get<TestField1>());
    CHECK(0x42 == msg.get<TestField2>());
    CHECK(0xd00d == msg.get<TestField3>());

    CHECK(0x8000ba11 == msg[0]);
    CHECK(0x0042d00d == msg[1]);
}

TEST_CASE("TestMessageDataDefaultConstruction", "[message]") {
    TestMsg msg{};

    CHECK(0x80 == msg.get<TestIdField>());
    CHECK(0x0 == msg.get<TestField1>());
    CHECK(0x0 == msg.get<TestField2>());
    CHECK(0x0 == msg.get<TestField3>());

    CHECK(0x80000000 == msg[0]);
    CHECK(0x0 == msg[1]);
}

TEST_CASE("TestMessageDataRawValuesConstruction", "[message]") {
    TestMsg msg{0x8000ba11, 0x0042d00d};

    CHECK(0x80 == msg.get<TestIdField>());
    CHECK(0xba11 == msg.get<TestField1>());
    CHECK(0x42 == msg.get<TestField2>());
    CHECK(0xd00d == msg.get<TestField3>());

    CHECK(0x8000ba11 == msg[0]);
    CHECK(0x0042d00d == msg[1]);
}

TEST_CASE("TestMessageDataRangeConstruction", "[message]") {
    TestMsg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(0xba11 == msg.get<TestField1>());
    CHECK(0x42 == msg.get<TestField2>());
    CHECK(0xd00d == msg.get<TestField3>());

    REQUIRE(msg.size() == 2u);
    CHECK(0x8000ba11 == msg[0]);
    CHECK(0x0042d00d == msg[1]);
}

TEST_CASE("TestMessageDataPartialRangeConstruction", "[message]") {
    TestMsg msg{std::array<uint32_t, 4>{0x8000ba11, 0x0042d00d, 0xffffffff,
                                        0xffffffff}};

    CHECK(0xba11 == msg.get<TestField1>());
    CHECK(0x42 == msg.get<TestField2>());
    CHECK(0xd00d == msg.get<TestField3>());

    REQUIRE(msg.size() == 2u);
    CHECK(0x8000ba11 == msg[0]);
    CHECK(0x0042d00d == msg[1]);
}

TEST_CASE("TestMessageDataPartialRangeConstructionLastBit", "[message]") {
    TestLastBitField msg{std::array<uint32_t, 4>{0x8000ba11, 0x2442d00d, 0xffffffff,
                                        0xffffffff}};

    CHECK(0xba11 == msg.get<TestField1>());
    CHECK(0x42 == msg.get<TestField2>());
    CHECK(0xd00d == msg.get<TestField3>());
    CHECK(0x24 == msg.get<TestField4>());

    REQUIRE(msg.size() == 2u);
    CHECK(0x8000ba11 == msg[0]);
    CHECK(0x2442d00d == msg[1]);
}

TEST_CASE("TestMessageDataConvertibleRangeConstruction", "[message]") {
    TestMsg msg{std::array<std::uint16_t, 2>{0xba11, 0xd00d}};

    CHECK(0xba11 == msg.get<TestField1>());
    CHECK(0x0 == msg.get<TestField2>());
    CHECK(0xd00d == msg.get<TestField3>());

    REQUIRE(msg.size() == 2u);
    CHECK(0x0000ba11 == msg[0]);
    CHECK(0x0000d00d == msg[1]);
}

TEST_CASE("EqualToMatcher", "[message]") {
    TestMsg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(TestIdField::equal_to<0x80>(msg));
    CHECK(TestField1::equal_to<0xba11>(msg));
    CHECK(TestField2::equal_to<0x42>(msg));
    CHECK(TestField3::equal_to<0xd00d>(msg));

    CHECK_FALSE(TestIdField::equal_to<0x0>(msg));
    CHECK_FALSE(TestField1::equal_to<0x0>(msg));
    CHECK_FALSE(TestField2::equal_to<0x0>(msg));
    CHECK_FALSE(TestField3::equal_to<0x0>(msg));
}

TEST_CASE("DefaultMatcher", "[message]") {
    TestMsg msg{std::array<uint32_t, 2>{0x0, 0x00}};

    CHECK(true == TestIdField::match_default(msg));
    CHECK(true == TestField1::match_default(msg));
    CHECK(true == TestField2::match_default(msg));
    CHECK(true == TestField3::match_default(msg));
}

TEST_CASE("DefaultMatcherNoMatch", "[message]") {
    TestMsg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(false == TestIdField::match_default(msg));
    CHECK(false == TestField1::match_default(msg));
    CHECK(false == TestField2::match_default(msg));
    CHECK(false == TestField3::match_default(msg));
}

TEST_CASE("InMatcher", "[message]") {
    TestMsg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK((TestIdField::in<0x80>(msg)));
    CHECK((TestField1::in<0xba11>(msg)));
    CHECK((TestField2::in<0x42>(msg)));
    CHECK((TestField3::in<0xd00d>(msg)));

    CHECK_FALSE((TestIdField::in<0x11>(msg)));
    CHECK_FALSE((TestField1::in<0x1111>(msg)));
    CHECK_FALSE((TestField2::in<0x11>(msg)));
    CHECK_FALSE((TestField3::in<0x1111>(msg)));

    CHECK((TestIdField::in<0x80, 0x0>(msg)));
    CHECK((TestField1::in<0xba11, 0x0>(msg)));
    CHECK((TestField2::in<0x42, 0x0>(msg)));
    CHECK((TestField3::in<0xd00d, 0x0>(msg)));

    CHECK_FALSE((TestIdField::in<0x11, 0x0>(msg)));
    CHECK_FALSE((TestField1::in<0x1111, 0x0>(msg)));
    CHECK_FALSE((TestField2::in<0x11, 0x0>(msg)));
    CHECK_FALSE((TestField3::in<0x1111, 0x0>(msg)));

    CHECK((TestIdField::in<0x80, 0x0, 0xE>(msg)));
    CHECK((TestField1::in<0xba11, 0x0, 0xEEEE>(msg)));
    CHECK((TestField2::in<0x42, 0x0, 0xEE>(msg)));
    CHECK((TestField3::in<0xd00d, 0x0, 0xEEEE>(msg)));

    CHECK_FALSE((TestIdField::in<0x11, 0x0, 0xE>(msg)));
    CHECK_FALSE((TestField1::in<0x1111, 0x0, 0xEEEE>(msg)));
    CHECK_FALSE((TestField2::in<0x11, 0x0, 0xEE>(msg)));
    CHECK_FALSE((TestField3::in<0x1111, 0x0, 0xEEEE>(msg)));
}

TEST_CASE("GreaterThanMatcher", "[message]") {
    TestMsg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK_FALSE(TestIdField::greater_than<0x80>(msg));
    CHECK_FALSE(TestField1::greater_than<0xba11>(msg));
    CHECK_FALSE(TestField2::greater_than<0x42>(msg));
    CHECK_FALSE(TestField3::greater_than<0xd00d>(msg));

    CHECK(TestIdField::greater_than<0x11>(msg));
    CHECK(TestField1::greater_than<0x1111>(msg));
    CHECK(TestField2::greater_than<0x11>(msg));
    CHECK(TestField3::greater_than<0x1111>(msg));
}

TEST_CASE("GreaterThanOrEqualToMatcher", "[message]") {
    TestMsg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK_FALSE(TestIdField::greater_than_or_equal_to<0xEE>(msg));
    CHECK_FALSE(TestField1::greater_than_or_equal_to<0xEEEE>(msg));
    CHECK_FALSE(TestField2::greater_than_or_equal_to<0xEE>(msg));
    CHECK_FALSE(TestField3::greater_than_or_equal_to<0xEEEE>(msg));

    CHECK(TestIdField::greater_than_or_equal_to<0x80>(msg));
    CHECK(TestField1::greater_than_or_equal_to<0xba11>(msg));
    CHECK(TestField2::greater_than_or_equal_to<0x42>(msg));
    CHECK(TestField3::greater_than_or_equal_to<0xd00d>(msg));

    CHECK(TestIdField::greater_than_or_equal_to<0x11>(msg));
    CHECK(TestField1::greater_than_or_equal_to<0x1111>(msg));
    CHECK(TestField2::greater_than_or_equal_to<0x11>(msg));
    CHECK(TestField3::greater_than_or_equal_to<0x1111>(msg));
}

TEST_CASE("LessThanMatcher", "[message]") {
    TestMsg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK_FALSE(TestIdField::less_than<0x80>(msg));
    CHECK_FALSE(TestField1::less_than<0xba11>(msg));
    CHECK_FALSE(TestField2::less_than<0x42>(msg));
    CHECK_FALSE(TestField3::less_than<0xd00d>(msg));

    CHECK(TestIdField::less_than<0xEE>(msg));
    CHECK(TestField1::less_than<0xEEEE>(msg));
    CHECK(TestField2::less_than<0xEE>(msg));
    CHECK(TestField3::less_than<0xEEEE>(msg));
}

TEST_CASE("LessThanOrEqualToMatcher", "[message]") {
    TestMsg msg{std::array<uint32_t, 2>{0x8000ba11, 0x0042d00d}};

    CHECK(TestIdField::less_than_or_equal_to<0xEE>(msg));
    CHECK(TestField1::less_than_or_equal_to<0xEEEE>(msg));
    CHECK(TestField2::less_than_or_equal_to<0xEE>(msg));
    CHECK(TestField3::less_than_or_equal_to<0xEEEE>(msg));

    CHECK(TestIdField::less_than_or_equal_to<0x80>(msg));
    CHECK(TestField1::less_than_or_equal_to<0xba11>(msg));
    CHECK(TestField2::less_than_or_equal_to<0x42>(msg));
    CHECK(TestField3::less_than_or_equal_to<0xd00d>(msg));

    CHECK_FALSE(TestIdField::less_than_or_equal_to<0x11>(msg));
    CHECK_FALSE(TestField1::less_than_or_equal_to<0x1111>(msg));
    CHECK_FALSE(TestField2::less_than_or_equal_to<0x11>(msg));
    CHECK_FALSE(TestField3::less_than_or_equal_to<0x1111>(msg));
}

} // namespace msg
