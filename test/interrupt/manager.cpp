#include <cib/cib.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <conc/concurrency.hpp>

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;

namespace interrupt {
class MockCallback {
  public:
    MOCK_METHOD(void, init, ());
    MOCK_METHOD(void, init,
                (std::size_t irq_number, std::size_t priorityLevel));
    MOCK_METHOD(void, run, (std::size_t irq_number));
    MOCK_METHOD(void, status, (std::size_t irq_number));

    MOCK_METHOD(void, write, (int fieldIndex, uint32_t value));
    MOCK_METHOD(uint32_t, read, (int fieldIndex));
};

MockCallback *callbackPtr;

struct MockIrqImpl {
    static void init() { callbackPtr->init(); }

    template <bool enable, int irq_number, int priorityLevel>
    inline static void irqInit() {
        if constexpr (enable) {
            callbackPtr->init(irq_number, priorityLevel);
        }
    }

    template <typename StatusPolicy, typename Callable>
    static void run(std::size_t irq_number,
                    Callable interrupt_service_routine) {
        StatusPolicy::run([&] { callbackPtr->status(irq_number); },
                          [&] {
                              callbackPtr->run(irq_number);
                              interrupt_service_routine();
                          });
    }
};

class InterruptManagerTest : public ::testing::Test {
  protected:
    MockCallback callback;

    void SetUp() override { callbackPtr = &callback; }
};

static auto constexpr msg_handler = flow::action("msg_handler"_sc, [] {
    // do nothing
});

static auto constexpr rsp_handler = flow::action("rsp_handler"_sc, [] {
    // do nothing
});

static auto constexpr timer_action = flow::action("timer_action"_sc, [] {
    // do nothing
});

class msg_handler_irq : public irq_flow<decltype("msg_handler_irq"_sc)> {};
class rsp_handler_irq : public irq_flow<decltype("rsp_handler_irq"_sc)> {};
class timer_irq : public irq_flow<decltype("timer_irq"_sc)> {};

class i2c_handler_irq : public irq_flow<decltype("i2c_handler_irq"_sc)> {};
class spi_handler_irq : public irq_flow<decltype("spi_handler_irq"_sc)> {};
class can_handler_irq : public irq_flow<decltype("can_handler_irq"_sc)> {};

struct test_resource_alpha {};
struct test_resource_beta {};

template <typename FieldType> struct field_value_t {
    constexpr static auto id = FieldType::id;

    uint32_t value;
};

template <int Id, typename Reg, typename Name, int Msb, int Lsb>
struct mock_field_t {
    constexpr static auto id = Id;
    using RegisterType = Reg;
    using DataType = uint32_t;

    constexpr static Reg get_register() { return {}; }

    constexpr static DataType get_mask() {
        return ((1 << (Msb + 1)) - 1) - ((1 << Lsb) - 1);
    }

    constexpr auto operator()(uint32_t value) const {
        return field_value_t<mock_field_t>{value};
    }
};

template <int Id, typename Reg, typename Name> struct mock_register_t {
    constexpr static auto id = Id;
    using RegisterType = Reg;
    using DataType = uint32_t;

    constexpr static Reg get_register() { return {}; }

    constexpr static mock_field_t<Id, Reg, decltype("raw"_sc), 31, 0> raw{};
};

template <typename FieldType> struct read_op_t {
    FieldType field;

    auto operator()() const { return callbackPtr->read(field.id); }
};

template <typename FieldType> constexpr auto read(FieldType field) {
    return read_op_t<FieldType>{field};
}

template <typename... ValueTypes> struct write_op_t {
    cib::tuple<ValueTypes...> values;

    void operator()() const {
        cib::for_each(
            [](auto value) { callbackPtr->write(value.id, value.value); },
            values);
    }
};

template <typename... ValueTypes> constexpr auto write(ValueTypes... values) {
    return write_op_t<ValueTypes...>{cib::make_tuple(values...)};
}

template <typename... FieldTypes> constexpr auto clear(FieldTypes...) {
    return write_op_t<field_value_t<FieldTypes>...>{
        cib::make_tuple(field_value_t<FieldTypes>{0}...)};
}

template <typename... OpTypes> constexpr void apply(OpTypes... ops) {
    (ops(), ...);
}

template <typename FieldType>
constexpr auto apply(read_op_t<FieldType> read_op) {
    return read_op();
}

struct int_sts_reg_t
    : mock_register_t<0, int_sts_reg_t, decltype("int_sts"_sc)> {};
struct packet_avail_sts_field_t
    : mock_field_t<1, int_sts_reg_t, decltype("packet_avail_sts"_sc), 1, 1> {};
struct rsp_avail_sts_field_t
    : mock_field_t<2, int_sts_reg_t, decltype("rsp_avail_sts"_sc), 0, 0> {};

struct int_en_reg_t : mock_register_t<3, int_en_reg_t, decltype("int_en"_sc)> {
};
struct packet_avail_en_field_t
    : mock_field_t<4, int_en_reg_t, decltype("packet_avail_en"_sc), 1, 1> {};
struct rsp_avail_en_field_t
    : mock_field_t<5, int_en_reg_t, decltype("rsp_avail_en"_sc), 0, 0> {};

#define EXPECT_WRITE(FIELD_TYPE, VALUE)                                        \
    EXPECT_CALL(callback, write(FIELD_TYPE::id, VALUE))
#define EXPECT_READ(FIELD_TYPE) EXPECT_CALL(callback, read(FIELD_TYPE::id))

struct BasicBuilder {
    using Config = root<
        MockIrqImpl,

        shared_irq<33, 0, policies<clear_status_first>,
                   sub_irq<packet_avail_en_field_t, packet_avail_sts_field_t,
                           msg_handler_irq,
                           policies<required_resources<test_resource_alpha>>>,
                   sub_irq<rsp_avail_en_field_t, rsp_avail_sts_field_t,
                           rsp_handler_irq,
                           policies<required_resources<test_resource_alpha,
                                                       test_resource_beta>>>>,
        irq<38, 0, timer_irq, policies<>>>;

    using Dynamic = dynamic_controller<Config, test::ConcurrencyPolicy>;

    struct test_service : interrupt::service<Config, test::ConcurrencyPolicy> {
    };

    struct test_project {
        static constexpr auto config = cib::config(
            cib::exports<test_service>,
            interrupt::extend<test_service, msg_handler_irq>(msg_handler),
            interrupt::extend<test_service, rsp_handler_irq>(rsp_handler),
            interrupt::extend<test_service, timer_irq>(timer_action));
    };

    CIB_CONSTINIT static inline cib::nexus<test_project> test_nexus{};
    CIB_CONSTINIT static inline auto &manager =
        test_nexus.service<test_service>;
};

TEST_F(InterruptManagerTest, BasicManagerInit) {
    constexpr auto &manager = BasicBuilder::manager;

    EXPECT_CALL(callback, init()).Times(1);
    EXPECT_CALL(callback, init(33, 0)).Times(1);
    EXPECT_CALL(callback, init(38, 0)).Times(1);

    EXPECT_WRITE(int_en_reg_t, 3);

    manager.init();

    EXPECT_EQ(38, manager.max_irq());
}

TEST_F(InterruptManagerTest, BasicManagerIrqRun) {
    constexpr auto &manager = BasicBuilder::manager;

    EXPECT_CALL(callback, run(38)).Times(1);

    manager.run<38>();
}

TEST_F(InterruptManagerTest, BasicManagerSharedIrqRun) {
    constexpr auto &manager = BasicBuilder::manager;

    EXPECT_READ(packet_avail_en_field_t).WillRepeatedly(Return(1));
    EXPECT_READ(rsp_avail_en_field_t).WillRepeatedly(Return(0));

    EXPECT_READ(packet_avail_sts_field_t).WillOnce(Return(1));
    EXPECT_WRITE(packet_avail_sts_field_t, 0);

    EXPECT_CALL(callback, run(33)).Times(1);

    manager.run<33>();
}

TEST_F(InterruptManagerTest, BasicManagerSharedIrqRunAllEnabled) {
    constexpr auto &manager = BasicBuilder::manager;

    EXPECT_READ(packet_avail_en_field_t).WillRepeatedly(Return(1));
    EXPECT_READ(rsp_avail_en_field_t).WillRepeatedly(Return(1));

    EXPECT_READ(packet_avail_sts_field_t).WillOnce(Return(1));
    EXPECT_READ(rsp_avail_sts_field_t).WillOnce(Return(1));

    EXPECT_WRITE(packet_avail_sts_field_t, 0);
    EXPECT_WRITE(rsp_avail_sts_field_t, 0);

    EXPECT_CALL(callback, run(33)).Times(1);

    manager.run<33>();
}

TEST_F(InterruptManagerTest, BasicManagerSharedIrqRunAllDisabled) {
    constexpr auto &manager = BasicBuilder::manager;

    EXPECT_READ(packet_avail_en_field_t).WillRepeatedly(Return(0));
    EXPECT_READ(rsp_avail_en_field_t).WillRepeatedly(Return(0));

    EXPECT_CALL(callback, run(33)).Times(1);

    manager.run<33>();
}

struct NoIsrBuilder {
    // Configure Interrupts
    using Config = root<
        MockIrqImpl,

        shared_irq<33, 0, policies<clear_status_first>,
                   sub_irq<packet_avail_en_field_t, packet_avail_sts_field_t,
                           msg_handler_irq, policies<>>,
                   sub_irq<rsp_avail_en_field_t, rsp_avail_sts_field_t,
                           rsp_handler_irq, policies<>>>,
        irq<38, 0, timer_irq, policies<>>>;

    using Dynamic = dynamic_controller<Config, test::ConcurrencyPolicy>;

    struct test_service : interrupt::service<Config, test::ConcurrencyPolicy> {
    };

    struct test_project {
        static constexpr auto config = cib::config(cib::exports<test_service>);
    };

    CIB_CONSTINIT static inline cib::nexus<test_project> test_nexus{};
    CIB_CONSTINIT static inline auto &manager =
        test_nexus.service<test_service>;
};

TEST_F(InterruptManagerTest, NoIsrInit) {
    constexpr auto &manager = NoIsrBuilder::manager;

    EXPECT_READ(packet_avail_sts_field_t).Times(0);
    EXPECT_READ(rsp_avail_sts_field_t).Times(0);

    EXPECT_WRITE(packet_avail_sts_field_t, _).Times(0);
    EXPECT_WRITE(rsp_avail_sts_field_t, _).Times(0);

    // MCU interrupts always get enabled for shared interrupts
    EXPECT_CALL(callback, init(33, 0)).Times(1);

    // single MCU interrupts get disabled if unused
    EXPECT_CALL(callback, init(38, 0)).Times(0);

    manager.init();
}

struct ClearStatusFirstBuilder {
    // Configure Interrupts
    using Config =
        root<MockIrqImpl, irq<38, 0, timer_irq, policies<clear_status_first>>>;

    using Dynamic = dynamic_controller<Config, test::ConcurrencyPolicy>;

    struct test_service : interrupt::service<Config, test::ConcurrencyPolicy> {
    };

    struct test_project {
        static constexpr auto config = cib::config(
            cib::exports<test_service>,
            interrupt::extend<test_service, timer_irq>(timer_action));
    };

    CIB_CONSTINIT static inline cib::nexus<test_project> test_nexus{};
    CIB_CONSTINIT static inline auto &manager =
        test_nexus.service<test_service>;
};

TEST_F(InterruptManagerTest, ClearStatusFirstTest) {
    constexpr auto &manager = ClearStatusFirstBuilder::manager;

    {
        testing::InSequence s;
        EXPECT_CALL(callback, init(38, 0)).Times(1);
        EXPECT_CALL(callback, status(38)).Times(1);
        EXPECT_CALL(callback, run(38)).Times(1);
    }

    manager.init();
    manager.run<38>();
}

struct DontClearStatusBuilder {
    // Configure Interrupts
    using Config =
        root<MockIrqImpl, irq<38, 0, timer_irq, policies<dont_clear_status>>>;

    using Dynamic = dynamic_controller<Config, test::ConcurrencyPolicy>;

    struct test_service : interrupt::service<Config, test::ConcurrencyPolicy> {
    };

    struct test_project {
        static constexpr auto config = cib::config(
            cib::exports<test_service>,
            interrupt::extend<test_service, timer_irq>(timer_action));
    };

    CIB_CONSTINIT static inline cib::nexus<test_project> test_nexus{};
    CIB_CONSTINIT static inline auto &manager =
        test_nexus.service<test_service>;
};

TEST_F(InterruptManagerTest, DontClearStatusTest) {
    constexpr auto &manager = DontClearStatusBuilder::manager;

    EXPECT_CALL(callback, init(38, 0)).Times(1);
    EXPECT_CALL(callback, status(38)).Times(0);
    EXPECT_CALL(callback, run(38)).Times(1);

    manager.init();
    manager.run<38>();
}

TEST_F(InterruptManagerTest, DynamicInterruptEnable) {
    BasicBuilder::Dynamic::disable<msg_handler_irq, rsp_handler_irq>();

    EXPECT_WRITE(int_en_reg_t, 2);

    BasicBuilder::Dynamic::enable<msg_handler_irq>();
}

TEST_F(InterruptManagerTest, DynamicInterruptEnableMulti) {
    EXPECT_WRITE(int_en_reg_t, 3);

    BasicBuilder::Dynamic::enable<msg_handler_irq, rsp_handler_irq>();
}

TEST_F(InterruptManagerTest, ResourceDisableEnable) {
    BasicBuilder::Dynamic::disable<msg_handler_irq, rsp_handler_irq>();

    InSequence s;

    EXPECT_WRITE(int_en_reg_t, 1);
    BasicBuilder::Dynamic::enable<rsp_handler_irq>();

    EXPECT_WRITE(int_en_reg_t, 0);
    BasicBuilder::Dynamic::turn_off_resource<test_resource_beta>();

    EXPECT_WRITE(int_en_reg_t, 1);
    BasicBuilder::Dynamic::turn_on_resource<test_resource_beta>();
}

TEST_F(InterruptManagerTest, ResourceDisableEnableMultiIrq) {
    InSequence s;

    EXPECT_WRITE(int_en_reg_t, 3);
    BasicBuilder::Dynamic::enable<rsp_handler_irq, msg_handler_irq>();

    EXPECT_WRITE(int_en_reg_t, 2);
    BasicBuilder::Dynamic::turn_off_resource<test_resource_beta>();

    EXPECT_WRITE(int_en_reg_t, 3);
    BasicBuilder::Dynamic::turn_on_resource<test_resource_beta>();
}

TEST_F(InterruptManagerTest, ResourceDisableEnableBigResource) {
    InSequence s;

    EXPECT_WRITE(int_en_reg_t, 3);
    BasicBuilder::Dynamic::enable<rsp_handler_irq, msg_handler_irq>();

    EXPECT_WRITE(int_en_reg_t, 0);
    BasicBuilder::Dynamic::turn_off_resource<test_resource_alpha>();

    EXPECT_WRITE(int_en_reg_t, 3);
    BasicBuilder::Dynamic::turn_on_resource<test_resource_alpha>();
}

TEST_F(InterruptManagerTest, ResourceDisableEnableMultiResource) {
    InSequence s;

    EXPECT_WRITE(int_en_reg_t, 3);
    BasicBuilder::Dynamic::enable<rsp_handler_irq, msg_handler_irq>();

    EXPECT_WRITE(int_en_reg_t, 2);
    BasicBuilder::Dynamic::turn_off_resource<test_resource_beta>();

    EXPECT_WRITE(int_en_reg_t, 0);
    BasicBuilder::Dynamic::turn_off_resource<test_resource_alpha>();

    EXPECT_WRITE(int_en_reg_t, 2);
    BasicBuilder::Dynamic::turn_on_resource<test_resource_alpha>();

    EXPECT_WRITE(int_en_reg_t, 3);
    BasicBuilder::Dynamic::turn_on_resource<test_resource_beta>();
}

static auto constexpr bscan =
    flow::action("bscan"_sc, [] { callbackPtr->run(0xba5eba11); });

struct hwio_int_sts_field_t
    : mock_field_t<6, int_sts_reg_t, decltype("hwio_int_sts_field_t"_sc), 2,
                   2> {};
struct i2c_int_sts_field_t
    : mock_field_t<7, int_sts_reg_t, decltype("i2c_int_sts_field_t"_sc), 3, 3> {
};
struct spi_int_sts_field_t
    : mock_field_t<8, int_sts_reg_t, decltype("spi_int_sts_field_t"_sc), 4, 4> {
};
struct can_int_sts_field_t
    : mock_field_t<9, int_sts_reg_t, decltype("can_int_sts_field_t"_sc), 5, 5> {
};

struct hwio_int_en_field_t
    : mock_field_t<10, int_en_reg_t, decltype("hwio_int_en_field_t"_sc), 2, 2> {
};
struct i2c_int_en_field_t
    : mock_field_t<11, int_en_reg_t, decltype("i2c_int_en_field_t"_sc), 3, 3> {
};
struct spi_int_en_field_t
    : mock_field_t<12, int_en_reg_t, decltype("spi_int_en_field_t"_sc), 4, 4> {
};
struct can_int_en_field_t
    : mock_field_t<13, int_en_reg_t, decltype("can_int_en_field_t"_sc), 5, 5> {
};

struct SharedSubIrqTest {
    // Configure Interrupts
    using Config = root<
        MockIrqImpl,

        shared_irq<33, 0, policies<clear_status_first>,
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
        irq<38, 0, timer_irq, policies<>>>;

    using Dynamic = dynamic_controller<Config, test::ConcurrencyPolicy>;

    struct test_service : interrupt::service<Config, test::ConcurrencyPolicy> {
    };

    struct test_project {
        static constexpr auto config = cib::config(
            cib::exports<test_service>,
            interrupt::extend<test_service, i2c_handler_irq>(bscan));
    };

    CIB_CONSTINIT static inline cib::nexus<test_project> test_nexus{};
    CIB_CONSTINIT static inline auto &manager =
        test_nexus.service<test_service>;
};

TEST_F(InterruptManagerTest, BasicManagerSharedSubIrqRun) {
    constexpr auto &manager = SharedSubIrqTest::manager;

    EXPECT_READ(hwio_int_en_field_t).WillRepeatedly(Return(1));
    EXPECT_READ(i2c_int_en_field_t).WillRepeatedly(Return(1));

    EXPECT_READ(hwio_int_sts_field_t).WillOnce(Return(1));
    EXPECT_READ(i2c_int_sts_field_t).WillOnce(Return(1));

    EXPECT_WRITE(hwio_int_sts_field_t, 0);
    EXPECT_WRITE(i2c_int_sts_field_t, 0);

    EXPECT_CALL(callback, status(33)).Times(1);
    EXPECT_CALL(callback, run(33)).Times(1);
    EXPECT_CALL(callback, run(0xba5eba11)).Times(1);

    manager.run<33>();
}
} // namespace interrupt
