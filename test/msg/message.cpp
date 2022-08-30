#include <catch2/catch_test_macros.hpp>

#include <msg/Message.hpp>


namespace {
    using TestIdField = Field<
        decltype("TestIdField"_sc),
        0, 31, 24,
        std::uint32_t>;

    using TestField1 = Field<
        decltype("TestField1"_sc),
        0, 15, 0,
        std::uint32_t>;

    using TestField2 = Field<
        decltype("TestField2"_sc),
        1, 23, 16,
        std::uint32_t>;

    using TestField3 = Field<
        decltype("TestField3"_sc),
        1, 15, 0,
        std::uint32_t>;

    using TestMsg =
        MessageBase<
            decltype("TestMsg"_sc),
            4, 2,
            TestIdField::WithRequired<0x80>,
            TestField1,
            TestField2,
            TestField3>;

    TEST_CASE("TestMessageDataFieldConstruction", "[message]") {
        TestMsg msg{
            TestField1{0xba11},
            TestField2{0x42},
            TestField3{0xd00d}};

        REQUIRE(0xba11 == msg.get<TestField1>());
        REQUIRE(0x42 == msg.get<TestField2>());
        REQUIRE(0xd00d == msg.get<TestField3>());

        REQUIRE(0x8000ba11 == msg.data[0]);
        REQUIRE(0x0042d00d == msg.data[1]);

        INFO("msg = {}", msg.describe());
    }

    TEST_CASE("TestMessageDataFieldDefaultConstruction", "[message]") {
        TestMsg msg{};

        REQUIRE(0x80 == msg.get<TestIdField>());
        REQUIRE(0x0 == msg.get<TestField1>());
        REQUIRE(0x0 == msg.get<TestField2>());
        REQUIRE(0x0 == msg.get<TestField3>());

        REQUIRE(0x80000000 == msg.data[0]);
        REQUIRE(0x0 ==        msg.data[1]);

        INFO("msg = {}", msg.describe());
    }

    TEST_CASE("TestMessageDataFieldInitializerConstruction", "[message]") {
        TestMsg msg{0x8000ba11, 0x0042d00d};

        REQUIRE(0x80 == msg.get<TestIdField>());
        REQUIRE(0xba11 == msg.get<TestField1>());
        REQUIRE(0x42 == msg.get<TestField2>());
        REQUIRE(0xd00d == msg.get<TestField3>());

        REQUIRE(0x8000ba11 == msg.data[0]);
        REQUIRE(0x0042d00d == msg.data[1]);

        INFO("msg = {}", msg.describe());
    }

    TEST_CASE("TestMessageDataDWordConstruction", "[message]") {
        TestMsg msg{std::array<uint32_t, 2>{
            0x8000ba11,
            0x0042d00d}};

        REQUIRE(0xba11 == msg.get<TestField1>());
        REQUIRE(0x42 == msg.get<TestField2>());
        REQUIRE(0xd00d == msg.get<TestField3>());

        REQUIRE(0x8000ba11 == msg.data[0]);
        REQUIRE(0x0042d00d == msg.data[1]);

        INFO("msg = {}", msg.describe());
    }

    TEST_CASE("EqualToMatcher", "[message]") {
        TestMsg msg{std::array<uint32_t, 2>{
            0x8000ba11,
            0x0042d00d}};

        REQUIRE(TestIdField::equalTo<0x80>(msg));
        REQUIRE(TestField1::equalTo<0xba11>(msg));
        REQUIRE(TestField2::equalTo<0x42>(msg));
        REQUIRE(TestField3::equalTo<0xd00d>(msg));

        REQUIRE_FALSE(TestIdField::equalTo<0x0>(msg));
        REQUIRE_FALSE(TestField1::equalTo<0x0>(msg));
        REQUIRE_FALSE(TestField2::equalTo<0x0>(msg));
        REQUIRE_FALSE(TestField3::equalTo<0x0>(msg));
    }

    TEST_CASE("DefaultMatcher", "[message]") {
        TestMsg msg{std::array<uint32_t, 2>{
            0x0,
            0x00}};

        REQUIRE(true == TestIdField::matchDefault(msg));
        REQUIRE(true == TestField1::matchDefault(msg));
        REQUIRE(true == TestField2::matchDefault(msg));
        REQUIRE(true == TestField3::matchDefault(msg));
    }

    TEST_CASE("DefaultMatcherNoMatch", "[message]") {
        TestMsg msg{std::array<uint32_t, 2>{
            0x8000ba11,
            0x0042d00d}};

        REQUIRE(false == TestIdField::matchDefault(msg));
        REQUIRE(false == TestField1::matchDefault(msg));
        REQUIRE(false == TestField2::matchDefault(msg));
        REQUIRE(false == TestField3::matchDefault(msg));
    }

    TEST_CASE("InMatcher", "[message]") {
        TestMsg msg{std::array<uint32_t, 2>{
            0x8000ba11,
            0x0042d00d}};

        REQUIRE((TestIdField::in<0x80>(msg)));
        REQUIRE((TestField1::in<0xba11>(msg)));
        REQUIRE((TestField2::in<0x42>(msg)));
        REQUIRE((TestField3::in<0xd00d>(msg)));

        REQUIRE_FALSE((TestIdField::in<0x11>(msg)));
        REQUIRE_FALSE((TestField1::in<0x1111>(msg)));
        REQUIRE_FALSE((TestField2::in<0x11>(msg)));
        REQUIRE_FALSE((TestField3::in<0x1111>(msg)));

        REQUIRE((TestIdField::in<0x80, 0x0>(msg)));
        REQUIRE((TestField1::in<0xba11, 0x0>(msg)));
        REQUIRE((TestField2::in<0x42, 0x0>(msg)));
        REQUIRE((TestField3::in<0xd00d, 0x0>(msg)));

        REQUIRE_FALSE((TestIdField::in<0x11, 0x0>(msg)));
        REQUIRE_FALSE((TestField1::in<0x1111, 0x0>(msg)));
        REQUIRE_FALSE((TestField2::in<0x11, 0x0>(msg)));
        REQUIRE_FALSE((TestField3::in<0x1111, 0x0>(msg)));

        REQUIRE((TestIdField::in<0x80, 0x0, 0xE>(msg)));
        REQUIRE((TestField1::in<0xba11, 0x0, 0xEEEE>(msg)));
        REQUIRE((TestField2::in<0x42, 0x0, 0xEE>(msg)));
        REQUIRE((TestField3::in<0xd00d, 0x0, 0xEEEE>(msg)));

        REQUIRE_FALSE((TestIdField::in<0x11, 0x0, 0xE>(msg)));
        REQUIRE_FALSE((TestField1::in<0x1111, 0x0, 0xEEEE>(msg)));
        REQUIRE_FALSE((TestField2::in<0x11, 0x0, 0xEE>(msg)));
        REQUIRE_FALSE((TestField3::in<0x1111, 0x0, 0xEEEE>(msg)));

    }

    TEST_CASE("GreaterThanMatcher", "[message]") {
        TestMsg msg{std::array<uint32_t, 2>{
            0x8000ba11,
            0x0042d00d}};

        REQUIRE_FALSE(TestIdField::greaterThan<0x80>(msg));
        REQUIRE_FALSE(TestField1::greaterThan<0xba11>(msg));
        REQUIRE_FALSE(TestField2::greaterThan<0x42>(msg));
        REQUIRE_FALSE(TestField3::greaterThan<0xd00d>(msg));

        REQUIRE(TestIdField::greaterThan<0x11>(msg));
        REQUIRE(TestField1::greaterThan<0x1111>(msg));
        REQUIRE(TestField2::greaterThan<0x11>(msg));
        REQUIRE(TestField3::greaterThan<0x1111>(msg));
    }

    TEST_CASE("GreaterThanOrEqualToMatcher", "[message]") {
        TestMsg msg{std::array<uint32_t, 2>{
            0x8000ba11,
            0x0042d00d}};

        REQUIRE_FALSE(TestIdField::greaterThanOrEqualTo<0xEE>(msg));
        REQUIRE_FALSE(TestField1::greaterThanOrEqualTo<0xEEEE>(msg));
        REQUIRE_FALSE(TestField2::greaterThanOrEqualTo<0xEE>(msg));
        REQUIRE_FALSE(TestField3::greaterThanOrEqualTo<0xEEEE>(msg));

        REQUIRE(TestIdField::greaterThanOrEqualTo<0x80>(msg));
        REQUIRE(TestField1::greaterThanOrEqualTo<0xba11>(msg));
        REQUIRE(TestField2::greaterThanOrEqualTo<0x42>(msg));
        REQUIRE(TestField3::greaterThanOrEqualTo<0xd00d>(msg));

        REQUIRE(TestIdField::greaterThanOrEqualTo<0x11>(msg));
        REQUIRE(TestField1::greaterThanOrEqualTo<0x1111>(msg));
        REQUIRE(TestField2::greaterThanOrEqualTo<0x11>(msg));
        REQUIRE(TestField3::greaterThanOrEqualTo<0x1111>(msg));
    }

    TEST_CASE("LessThanMatcher", "[message]") {
        TestMsg msg{std::array<uint32_t, 2>{
            0x8000ba11,
            0x0042d00d}};

        REQUIRE_FALSE(TestIdField::lessThan<0x80>(msg));
        REQUIRE_FALSE(TestField1::lessThan<0xba11>(msg));
        REQUIRE_FALSE(TestField2::lessThan<0x42>(msg));
        REQUIRE_FALSE(TestField3::lessThan<0xd00d>(msg));

        REQUIRE(TestIdField::lessThan<0xEE>(msg));
        REQUIRE(TestField1::lessThan<0xEEEE>(msg));
        REQUIRE(TestField2::lessThan<0xEE>(msg));
        REQUIRE(TestField3::lessThan<0xEEEE>(msg));
    }

    TEST_CASE("LessThanOrEqualToMatcher", "[message]") {
        TestMsg msg{std::array<uint32_t, 2>{
            0x8000ba11,
            0x0042d00d}};

        REQUIRE(TestIdField::lessThanOrEqualTo<0xEE>(msg));
        REQUIRE(TestField1::lessThanOrEqualTo<0xEEEE>(msg));
        REQUIRE(TestField2::lessThanOrEqualTo<0xEE>(msg));
        REQUIRE(TestField3::lessThanOrEqualTo<0xEEEE>(msg));

        REQUIRE(TestIdField::lessThanOrEqualTo<0x80>(msg));
        REQUIRE(TestField1::lessThanOrEqualTo<0xba11>(msg));
        REQUIRE(TestField2::lessThanOrEqualTo<0x42>(msg));
        REQUIRE(TestField3::lessThanOrEqualTo<0xd00d>(msg));

        REQUIRE_FALSE(TestIdField::lessThanOrEqualTo<0x11>(msg));
        REQUIRE_FALSE(TestField1::lessThanOrEqualTo<0x1111>(msg));
        REQUIRE_FALSE(TestField2::lessThanOrEqualTo<0x11>(msg));
        REQUIRE_FALSE(TestField3::lessThanOrEqualTo<0x1111>(msg));
    }

}
