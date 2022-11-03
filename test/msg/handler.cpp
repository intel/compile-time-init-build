#include <msg/handler.hpp>

#include <catch2/catch_test_macros.hpp>

namespace msg {
bool correctDispatch = false;

using TestIdField = field<decltype("TestIdField"_sc), 0, 31, 24, std::uint32_t>;

using TestField1 = field<decltype("TestField1"_sc), 0, 15, 0, std::uint32_t>;

using TestField2 = field<decltype("TestField2"_sc), 1, 23, 16, std::uint32_t>;

using TestField3 = field<decltype("TestField3"_sc), 1, 15, 0, std::uint32_t>;

using TestBaseMsg = message_data<4>;

using TestMsg =
    message_base<decltype("TestMsg"_sc), 4, 2, TestIdField::WithRequired<0x80>,
                 TestField1, TestField2, TestField3>;

using TestMsgFieldRequired = message_base<decltype("TestMsgFieldRequired"_sc),
                                          4, 2, TestIdField::WithRequired<0x44>,
                                          TestField1, TestField2, TestField3>;

TEST_CASE("TestMsgDispatch1", "[handler]") {
    static auto callback =
        msg::callback<TestBaseMsg>("TestCallback"_sc, match::always<true>,
                                   [](TestMsg msg) { correctDispatch = true; });

    static auto handler = msg::handler<TestBaseMsg, 1>{{&callback}};

    handler.handle({0x8000ba11, 0x0042d00d});

    REQUIRE(correctDispatch);
}

// TODO: test no match in handle
// TODO: test is_match

TEST_CASE("TestMsgDispatch2", "[handler]") {
    static auto callback1 = msg::callback<TestBaseMsg>(
        "TestCallback1"_sc, match::always<true>,

        // if the raw data matches requirements of TestMsg, execute this
        [&](TestMsg const &msg) { REQUIRE(false); });

    static auto callback2 = msg::callback<TestBaseMsg>(
        "TestCallback2"_sc, match::always<true>,

        // if the raw data matches requirements of
        // TestMsgFieldRequired, execute this
        [](TestMsgFieldRequired const &msg) { correctDispatch = true; });

    static auto handler =
        msg::handler<TestBaseMsg, 2>{{&callback1, &callback2}};

    handler.handle({0x4400ba11, 0x0042d00d});

    REQUIRE(correctDispatch);
}

TEST_CASE("TestMsgDispatchExtraArgs1", "[handler]") {
    static auto callback = msg::callback<TestBaseMsg, int>(
        "TestCallback"_sc, match::always<true>, [](TestMsg, int value) {
            correctDispatch = true;
            REQUIRE(value == 0xcafe);
        });

    static auto handler = msg::handler<TestBaseMsg, 1, int>{{&callback}};

    handler.handle({0x8000ba11, 0x0042d00d}, 0xcafe);

    REQUIRE(correctDispatch);
}
} // namespace msg
