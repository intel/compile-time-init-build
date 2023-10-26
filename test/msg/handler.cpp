#include <match/ops.hpp>
#include <msg/callback.hpp>
#include <msg/field.hpp>
#include <msg/handler.hpp>
#include <msg/message.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using namespace msg;

bool correctDispatch = false;

using TestIdField =
    field<"TestIdField", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;

using TestField1 =
    field<"TestField1", std::uint32_t>::located<at{0_dw, 15_msb, 0_lsb}>;

using TestField2 =
    field<"TestField2", std::uint32_t>::located<at{1_dw, 23_msb, 16_lsb}>;

using TestField3 =
    field<"TestField3", std::uint32_t>::located<at{1_dw, 15_msb, 0_lsb}>;

using TestBaseMsg = message_data<2>;

using TestMsg =
    message_base<decltype("TestMsg"_sc), 2, TestIdField::WithRequired<0x80>,
                 TestField1, TestField2, TestField3>;

using TestMsgMultiCb =
    message_base<decltype("TestMsg"_sc), 2, TestIdField::WithRequired<0x81>,
                 TestField1, TestField2, TestField3>;

using TestMsgFieldRequired = message_base<decltype("TestMsgFieldRequired"_sc),
                                          2, TestIdField::WithRequired<0x44>,
                                          TestField1, TestField2, TestField3>;

enum class Opcode { A = 0x8, B = 0x9, C = 0xA };

using TestOpField =
    field<"TestOpField", Opcode>::located<at{0_dw, 27_msb, 24_lsb}>;

using TestMsgOp = message_base<decltype("TestMsg"_sc), 2,
                               TestOpField::WithIn<Opcode::A, Opcode::B>,
                               TestField1, TestField2>;
} // namespace

TEST_CASE("TestMsgDispatch1", "[handler]") {
    correctDispatch = false;

    static auto callback = msg::callback<TestBaseMsg>(
        "TestCallback"_sc, match::always,
        [](TestMsg const &) { correctDispatch = true; });

    auto callbacks = stdx::make_tuple(callback);

    static auto handler =
        msg::handler<decltype(callbacks), TestBaseMsg>{callbacks};

    handler.handle(TestBaseMsg{0x8000ba11, 0x0042d00d});

    REQUIRE(correctDispatch);
}

// TODO: test no match in handle
// TODO: test is_match

TEST_CASE("TestMsgDispatch2", "[handler]") {
    correctDispatch = false;

    static auto callback1 = msg::callback<TestBaseMsg>(
        "TestCallback1"_sc, match::always,

        // if the raw data matches requirements of TestMsg, execute this
        [&](TestMsg const &) { REQUIRE(false); });

    static auto callback2 = msg::callback<TestBaseMsg>(
        "TestCallback2"_sc, match::always,

        // if the raw data matches requirements of
        // TestMsgFieldRequired, execute this
        [](TestMsgFieldRequired const &) { correctDispatch = true; });

    auto callbacks = stdx::make_tuple(callback1, callback2);

    static auto handler =
        msg::handler<decltype(callbacks), TestBaseMsg>{callbacks};

    handler.handle(TestBaseMsg{0x4400ba11, 0x0042d00d});

    REQUIRE(correctDispatch);
}

TEST_CASE("TestMsgDispatchExtraArgs1", "[handler]") {
    correctDispatch = false;

    static auto callback = msg::callback<TestBaseMsg, int>(
        "TestCallback"_sc, match::always, [](TestMsg, int value) {
            correctDispatch = true;
            REQUIRE(value == 0xcafe);
        });

    auto callbacks = stdx::make_tuple(callback);

    static auto handler =
        msg::handler<decltype(callbacks), TestBaseMsg, int>{callbacks};

    handler.handle(TestBaseMsg{0x8000ba11, 0x0042d00d}, 0xcafe);

    REQUIRE(correctDispatch);
}

TEST_CASE("TestMsgWithinEnum", "[handler]") {
    auto handled = false;
    auto const callback =
        msg::callback<TestBaseMsg>("TestCallback"_sc, match::always,
                                   [&](TestMsgOp const &) { handled = true; });

    auto callbacks = stdx::make_tuple(callback);
    auto const handler =
        msg::handler<decltype(callbacks), TestBaseMsg>{callbacks};

    handler.handle(TestBaseMsg{0x0800ba11, 0x0042d00d});
    REQUIRE(handled);
}

TEST_CASE("TestMsgMultipleLambdaCallback", "[handler]") {
    {
        auto correct = false;
        auto const callback = msg::callback<TestBaseMsg>(
            "TestCallback"_sc, match::always, [](TestMsgMultiCb const &) {},
            [&](TestMsg const &) { correct = true; });
        auto callbacks = stdx::make_tuple(callback);
        auto const handler =
            msg::handler<decltype(callbacks), TestBaseMsg>{callbacks};

        handler.handle(TestBaseMsg{0x8000ba11, 0x0042d00d});
        REQUIRE(correct);
    }
    {
        auto correct = false;
        auto const callback = msg::callback<TestBaseMsg>(
            "TestCallback"_sc, match::always, [](TestMsg const &) {},
            [&](TestMsgMultiCb const &) { correct = true; });
        auto callbacks = stdx::make_tuple(callback);
        auto const handler =
            msg::handler<decltype(callbacks), TestBaseMsg>{callbacks};

        handler.handle(TestBaseMsg{0x8100ba11, 0x0042d00d});
        REQUIRE(correct);
    }
}
