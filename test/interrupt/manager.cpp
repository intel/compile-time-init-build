#include "common.hpp"

#include <cib/cib.hpp>

#include <stdx/concepts.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;

namespace interrupt {
class MockCallback {
  public:
    MOCK_METHOD(void, init, ());
    MOCK_METHOD(void, init, (irq_num_t irq_number, std::size_t priorityLevel));
    MOCK_METHOD(void, run, (irq_num_t irq_number));
    MOCK_METHOD(void, status, (irq_num_t irq_number));

    MOCK_METHOD(void, write, (int fieldIndex, std::uint32_t value));
    MOCK_METHOD(std::uint32_t, read, (int fieldIndex));
};

MockCallback *callbackPtr;

struct MockIrqImpl {
    static auto init() -> void { callbackPtr->init(); }

    template <bool Enable, irq_num_t IrqNumber, std::size_t Priority>
    static auto irq_init() -> void {
        if constexpr (Enable) {
            callbackPtr->init(IrqNumber, Priority);
        }
    }

    template <status_policy P>
    static auto run(irq_num_t irq_number, stdx::invocable auto const &isr)
        -> void {
        P::run([&] { callbackPtr->status(irq_number); },
               [&] {
                   callbackPtr->run(irq_number);
                   isr();
               });
    }
};

template <> inline auto injected_hal<> = MockIrqImpl{};

class InterruptManagerTest : public ::testing::Test {
  protected:
    MockCallback callback;

    void SetUp() override { callbackPtr = &callback; }
};

constexpr static auto msg_handler = flow::action<"msg_handler">([] {});
constexpr static auto rsp_handler = flow::action<"rsp_handler">([] {});
constexpr static auto timer_action = flow::action<"timer_action">([] {});

using msg_handler_irq = irq_flow<"msg_handler">;
using rsp_handler_irq = irq_flow<"rsp_handler">;
using timer_irq = irq_flow<"timer">;

using i2c_handler_irq = irq_flow<"i2c_handler">;
using spi_handler_irq = irq_flow<"spi_handler">;
using can_handler_irq = irq_flow<"can_handler">;

template <typename Field> constexpr auto read(Field) {
    return [] { return callbackPtr->read(Field::id); };
}

template <typename... Values> constexpr auto write(Values... values) {
    return [... values = values.value] {
        (callbackPtr->write(Values::id, values), ...);
    };
}

template <typename... Fields> constexpr auto clear(Fields...) {
    return [] { (callbackPtr->write(Fields::id, 0), ...); };
}

struct int_sts_reg_t : mock_register_t<0, int_sts_reg_t, "int_sts"> {};
using packet_avail_sts_field_t =
    mock_field_t<1, int_sts_reg_t, "packet_avail_sts", 1, 1>;
using rsp_avail_sts_field_t =
    mock_field_t<2, int_sts_reg_t, "rsp_avail_sts", 0, 0>;

struct int_en_reg_t : mock_register_t<3, int_en_reg_t, "int_en"> {};
using packet_avail_en_field_t =
    mock_field_t<4, int_en_reg_t, "packet_avail_en", 1, 1>;
using rsp_avail_en_field_t =
    mock_field_t<5, int_en_reg_t, "rsp_avail_en", 0, 0>;

#define EXPECT_WRITE(FIELD_TYPE, VALUE)                                        \
    EXPECT_CALL(callback, write(FIELD_TYPE::id, VALUE))
#define EXPECT_READ(FIELD_TYPE) EXPECT_CALL(callback, read(FIELD_TYPE::id))

struct BasicBuilder {
    using Config = root<
        shared_irq<33_irq, 0, policies<clear_status_first>,
                   sub_irq<packet_avail_en_field_t, packet_avail_sts_field_t,
                           msg_handler_irq, policies<>>,
                   sub_irq<rsp_avail_en_field_t, rsp_avail_sts_field_t,
                           rsp_handler_irq, policies<>>>,
        irq<38_irq, 0, timer_irq, policies<>>>;

    struct test_service : interrupt::service<Config> {};

    struct test_project {
        constexpr static auto config = cib::config(
            cib::exports<test_service>,
            interrupt::extend<test_service, msg_handler_irq>(msg_handler),
            interrupt::extend<test_service, rsp_handler_irq>(rsp_handler),
            interrupt::extend<test_service, timer_irq>(timer_action));
    };

    using nexus_t = cib::nexus<test_project>;
    CONSTINIT static inline nexus_t test_nexus{};
    CONSTINIT static inline auto &manager = nexus_t::service<test_service>;
};

TEST_F(InterruptManagerTest, BasicManagerInit) {
    constexpr auto &manager = BasicBuilder::manager;

    EXPECT_CALL(callback, init()).Times(1);
    EXPECT_CALL(callback, init(33_irq, 0)).Times(1);
    EXPECT_CALL(callback, init(38_irq, 0)).Times(1);

    EXPECT_WRITE(int_en_reg_t, 3);

    manager.init();

    EXPECT_EQ(38_irq, manager.max_irq());
}

TEST_F(InterruptManagerTest, BasicManagerIrqRun) {
    constexpr auto &manager = BasicBuilder::manager;

    EXPECT_CALL(callback, run(38_irq)).Times(1);

    manager.run<38_irq>();
}

TEST_F(InterruptManagerTest, BasicManagerSharedIrqRun) {
    constexpr auto &manager = BasicBuilder::manager;

    EXPECT_READ(packet_avail_en_field_t).WillRepeatedly(Return(1));
    EXPECT_READ(rsp_avail_en_field_t).WillRepeatedly(Return(0));

    EXPECT_READ(packet_avail_sts_field_t).WillOnce(Return(1));
    EXPECT_WRITE(packet_avail_sts_field_t, 0);

    EXPECT_CALL(callback, run(33_irq)).Times(1);

    manager.run<33_irq>();
}

TEST_F(InterruptManagerTest, BasicManagerSharedIrqRunAllEnabled) {
    constexpr auto &manager = BasicBuilder::manager;

    EXPECT_READ(packet_avail_en_field_t).WillRepeatedly(Return(1));
    EXPECT_READ(rsp_avail_en_field_t).WillRepeatedly(Return(1));

    EXPECT_READ(packet_avail_sts_field_t).WillOnce(Return(1));
    EXPECT_READ(rsp_avail_sts_field_t).WillOnce(Return(1));

    EXPECT_WRITE(packet_avail_sts_field_t, 0);
    EXPECT_WRITE(rsp_avail_sts_field_t, 0);

    EXPECT_CALL(callback, run(33_irq)).Times(1);

    manager.run<33_irq>();
}

TEST_F(InterruptManagerTest, BasicManagerSharedIrqRunAllDisabled) {
    constexpr auto &manager = BasicBuilder::manager;

    EXPECT_READ(packet_avail_en_field_t).WillRepeatedly(Return(0));
    EXPECT_READ(rsp_avail_en_field_t).WillRepeatedly(Return(0));

    EXPECT_CALL(callback, run(33_irq)).Times(1);

    manager.run<33_irq>();
}

struct NoIsrBuilder {
    // Configure Interrupts
    using Config = root<
        shared_irq<33_irq, 0, policies<clear_status_first>,
                   sub_irq<packet_avail_en_field_t, packet_avail_sts_field_t,
                           msg_handler_irq, policies<>>,
                   sub_irq<rsp_avail_en_field_t, rsp_avail_sts_field_t,
                           rsp_handler_irq, policies<>>>,
        irq<38_irq, 0, timer_irq, policies<>>>;

    struct test_service : interrupt::service<Config> {};

    struct test_project {
        constexpr static auto config = cib::config(cib::exports<test_service>);
    };

    using nexus_t = cib::nexus<test_project>;
    CONSTINIT static inline nexus_t test_nexus{};
    CONSTINIT static inline auto &manager = nexus_t::service<test_service>;
};

TEST_F(InterruptManagerTest, NoIsrInit) {
    constexpr auto &manager = NoIsrBuilder::manager;

    EXPECT_READ(packet_avail_sts_field_t).Times(0);
    EXPECT_READ(rsp_avail_sts_field_t).Times(0);

    EXPECT_WRITE(packet_avail_sts_field_t, _).Times(0);
    EXPECT_WRITE(rsp_avail_sts_field_t, _).Times(0);

    // MCU interrupts always get enabled for shared interrupts
    EXPECT_CALL(callback, init(33_irq, 0)).Times(1);

    // single MCU interrupts get disabled if unused
    EXPECT_CALL(callback, init(38_irq, 0)).Times(0);

    manager.init();
}

struct ClearStatusFirstBuilder {
    // Configure Interrupts
    using Config =
        root<irq<38_irq, 0, timer_irq, policies<clear_status_first>>>;

    struct test_service : interrupt::service<Config> {};

    struct test_project {
        constexpr static auto config = cib::config(
            cib::exports<test_service>,
            interrupt::extend<test_service, timer_irq>(timer_action));
    };

    using nexus_t = cib::nexus<test_project>;
    CONSTINIT static inline nexus_t test_nexus{};
    CONSTINIT static inline auto &manager = nexus_t::service<test_service>;
};

TEST_F(InterruptManagerTest, ClearStatusFirstTest) {
    constexpr auto &manager = ClearStatusFirstBuilder::manager;

    {
        testing::InSequence s;
        EXPECT_CALL(callback, init(38_irq, 0)).Times(1);
        EXPECT_CALL(callback, status(38_irq)).Times(1);
        EXPECT_CALL(callback, run(38_irq)).Times(1);
    }

    manager.init();
    manager.run<38_irq>();
}

struct DontClearStatusBuilder {
    // Configure Interrupts
    using Config = root<irq<38_irq, 0, timer_irq, policies<dont_clear_status>>>;

    struct test_service : interrupt::service<Config> {};

    struct test_project {
        constexpr static auto config = cib::config(
            cib::exports<test_service>,
            interrupt::extend<test_service, timer_irq>(timer_action));
    };

    using nexus_t = cib::nexus<test_project>;
    CONSTINIT static inline nexus_t test_nexus{};
    CONSTINIT static inline auto &manager = nexus_t::service<test_service>;
};

TEST_F(InterruptManagerTest, DontClearStatusTest) {
    constexpr auto &manager = DontClearStatusBuilder::manager;

    EXPECT_CALL(callback, init(38_irq, 0)).Times(1);
    EXPECT_CALL(callback, status(38_irq)).Times(0);
    EXPECT_CALL(callback, run(38_irq)).Times(1);

    manager.init();
    manager.run<38_irq>();
}

constexpr static auto bscan =
    flow::action<"bscan">([] { callbackPtr->run(0xba5eba11_irq); });

using hwio_int_sts_field_t =
    mock_field_t<6, int_sts_reg_t, "hwio_int_sts_field_t", 2, 2>;
using i2c_int_sts_field_t =
    mock_field_t<7, int_sts_reg_t, "i2c_int_sts_field_t", 3, 3>;
using spi_int_sts_field_t =
    mock_field_t<8, int_sts_reg_t, "spi_int_sts_field_t", 4, 4>;
using can_int_sts_field_t =
    mock_field_t<9, int_sts_reg_t, "can_int_sts_field_t", 5, 5>;

using hwio_int_en_field_t =
    mock_field_t<10, int_en_reg_t, "hwio_int_en_field_t", 2, 2>;
using i2c_int_en_field_t =
    mock_field_t<11, int_en_reg_t, "i2c_int_en_field_t", 3, 3>;
using spi_int_en_field_t =
    mock_field_t<12, int_en_reg_t, "spi_int_en_field_t", 4, 4>;
using can_int_en_field_t =
    mock_field_t<13, int_en_reg_t, "can_int_en_field_t", 5, 5>;

struct SharedSubIrqTest {
    // Configure Interrupts
    using Config = root<
        shared_irq<33_irq, 0, policies<clear_status_first>,
                   sub_irq<packet_avail_en_field_t, packet_avail_sts_field_t,
                           msg_handler_irq, policies<>>,
                   sub_irq<rsp_avail_en_field_t, rsp_avail_sts_field_t,
                           rsp_handler_irq, policies<>>,
                   shared_sub_irq<
                       hwio_int_en_field_t, hwio_int_sts_field_t, policies<>,
                       sub_irq<i2c_int_en_field_t, i2c_int_sts_field_t,
                               i2c_handler_irq, policies<>>,
                       sub_irq<spi_int_en_field_t, spi_int_sts_field_t,
                               spi_handler_irq, policies<>>,
                       sub_irq<can_int_en_field_t, can_int_sts_field_t,
                               can_handler_irq, policies<dont_clear_status>>>>,
        irq<38_irq, 0, timer_irq, policies<>>>;

    struct test_service : interrupt::service<Config> {};

    struct test_project {
        constexpr static auto config = cib::config(
            cib::exports<test_service>,
            interrupt::extend<test_service, i2c_handler_irq>(bscan));
    };

    using nexus_t = cib::nexus<test_project>;
    CONSTINIT static inline nexus_t test_nexus{};
    CONSTINIT static inline auto &manager = nexus_t::service<test_service>;
};

TEST_F(InterruptManagerTest, BasicManagerSharedSubIrqRun) {
    constexpr auto &manager = SharedSubIrqTest::manager;

    EXPECT_READ(hwio_int_en_field_t).WillRepeatedly(Return(1));
    EXPECT_READ(i2c_int_en_field_t).WillRepeatedly(Return(1));

    EXPECT_READ(hwio_int_sts_field_t).WillOnce(Return(1));
    EXPECT_READ(i2c_int_sts_field_t).WillOnce(Return(1));

    EXPECT_WRITE(hwio_int_sts_field_t, 0);
    EXPECT_WRITE(i2c_int_sts_field_t, 0);

    EXPECT_CALL(callback, status(33_irq)).Times(1);
    EXPECT_CALL(callback, run(33_irq)).Times(1);
    EXPECT_CALL(callback, run(0xba5eba11_irq)).Times(1);

    manager.run<33_irq>();
}
} // namespace interrupt
