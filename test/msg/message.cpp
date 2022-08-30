#include <catch2/catch_test_macros.hpp>

#include <msg/message.hpp>


namespace msg {
    using TestIdField = field<
        decltype("TestIdField"_sc),
        0, 31, 24,
        std::uint32_t>;

    using TestField1 = field<
        decltype("TestField1"_sc),
        0, 15, 0,
        std::uint32_t>;

    using TestField2 = field<
        decltype("TestField2"_sc),
        1, 23, 16,
        std::uint32_t>;

    using TestField3 = field<
        decltype("TestField3"_sc),
        1, 15, 0,
        std::uint32_t>;

    using TestMsg =
        message_base<
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

        REQUIRE(TestIdField::equal_to<0x80>(msg));
        REQUIRE(TestField1::equal_to<0xba11>(msg));
        REQUIRE(TestField2::equal_to<0x42>(msg));
        REQUIRE(TestField3::equal_to<0xd00d>(msg));

        REQUIRE_FALSE(TestIdField::equal_to<0x0>(msg));
        REQUIRE_FALSE(TestField1::equal_to<0x0>(msg));
        REQUIRE_FALSE(TestField2::equal_to<0x0>(msg));
        REQUIRE_FALSE(TestField3::equal_to<0x0>(msg));
    }

    TEST_CASE("DefaultMatcher", "[message]") {
        TestMsg msg{std::array<uint32_t, 2>{
            0x0,
            0x00}};

        REQUIRE(true == TestIdField::match_default(msg));
        REQUIRE(true == TestField1::match_default(msg));
        REQUIRE(true == TestField2::match_default(msg));
        REQUIRE(true == TestField3::match_default(msg));
    }

    TEST_CASE("DefaultMatcherNoMatch", "[message]") {
        TestMsg msg{std::array<uint32_t, 2>{
            0x8000ba11,
            0x0042d00d}};

        REQUIRE(false == TestIdField::match_default(msg));
        REQUIRE(false == TestField1::match_default(msg));
        REQUIRE(false == TestField2::match_default(msg));
        REQUIRE(false == TestField3::match_default(msg));
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

        REQUIRE_FALSE(TestIdField::greater_than<0x80>(msg));
        REQUIRE_FALSE(TestField1::greater_than<0xba11>(msg));
        REQUIRE_FALSE(TestField2::greater_than<0x42>(msg));
        REQUIRE_FALSE(TestField3::greater_than<0xd00d>(msg));

        REQUIRE(TestIdField::greater_than<0x11>(msg));
        REQUIRE(TestField1::greater_than<0x1111>(msg));
        REQUIRE(TestField2::greater_than<0x11>(msg));
        REQUIRE(TestField3::greater_than<0x1111>(msg));
    }

    TEST_CASE("GreaterThanOrEqualToMatcher", "[message]") {
        TestMsg msg{std::array<uint32_t, 2>{
            0x8000ba11,
            0x0042d00d}};

        REQUIRE_FALSE(TestIdField::greater_than_or_equal_to<0xEE>(msg));
        REQUIRE_FALSE(TestField1::greater_than_or_equal_to<0xEEEE>(msg));
        REQUIRE_FALSE(TestField2::greater_than_or_equal_to<0xEE>(msg));
        REQUIRE_FALSE(TestField3::greater_than_or_equal_to<0xEEEE>(msg));

        REQUIRE(TestIdField::greater_than_or_equal_to<0x80>(msg));
        REQUIRE(TestField1::greater_than_or_equal_to<0xba11>(msg));
        REQUIRE(TestField2::greater_than_or_equal_to<0x42>(msg));
        REQUIRE(TestField3::greater_than_or_equal_to<0xd00d>(msg));

        REQUIRE(TestIdField::greater_than_or_equal_to<0x11>(msg));
        REQUIRE(TestField1::greater_than_or_equal_to<0x1111>(msg));
        REQUIRE(TestField2::greater_than_or_equal_to<0x11>(msg));
        REQUIRE(TestField3::greater_than_or_equal_to<0x1111>(msg));
    }

    TEST_CASE("LessThanMatcher", "[message]") {
        TestMsg msg{std::array<uint32_t, 2>{
            0x8000ba11,
            0x0042d00d}};

        REQUIRE_FALSE(TestIdField::less_than<0x80>(msg));
        REQUIRE_FALSE(TestField1::less_than<0xba11>(msg));
        REQUIRE_FALSE(TestField2::less_than<0x42>(msg));
        REQUIRE_FALSE(TestField3::less_than<0xd00d>(msg));

        REQUIRE(TestIdField::less_than<0xEE>(msg));
        REQUIRE(TestField1::less_than<0xEEEE>(msg));
        REQUIRE(TestField2::less_than<0xEE>(msg));
        REQUIRE(TestField3::less_than<0xEEEE>(msg));
    }

    TEST_CASE("LessThanOrEqualToMatcher", "[message]") {
        TestMsg msg{std::array<uint32_t, 2>{
            0x8000ba11,
            0x0042d00d}};

        REQUIRE(TestIdField::less_than_or_equal_to<0xEE>(msg));
        REQUIRE(TestField1::less_than_or_equal_to<0xEEEE>(msg));
        REQUIRE(TestField2::less_than_or_equal_to<0xEE>(msg));
        REQUIRE(TestField3::less_than_or_equal_to<0xEEEE>(msg));

        REQUIRE(TestIdField::less_than_or_equal_to<0x80>(msg));
        REQUIRE(TestField1::less_than_or_equal_to<0xba11>(msg));
        REQUIRE(TestField2::less_than_or_equal_to<0x42>(msg));
        REQUIRE(TestField3::less_than_or_equal_to<0xd00d>(msg));

        REQUIRE_FALSE(TestIdField::less_than_or_equal_to<0x11>(msg));
        REQUIRE_FALSE(TestField1::less_than_or_equal_to<0x1111>(msg));
        REQUIRE_FALSE(TestField2::less_than_or_equal_to<0x11>(msg));
        REQUIRE_FALSE(TestField3::less_than_or_equal_to<0x1111>(msg));
    }

}
