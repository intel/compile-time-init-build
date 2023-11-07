#include <lookup/input.hpp>
#include <lookup/lookup.hpp>
#include <msg/callback.hpp>
#include <msg/field.hpp>
#include <msg/indexed_handler.hpp>
#include <msg/message.hpp>

#include <stdx/bitset.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>

#define CX_VALUE(...)                                                          \
    [] {                                                                       \
        struct {                                                               \
            CONSTEVAL auto operator()() const noexcept { return __VA_ARGS__; } \
            using cx_value_t [[maybe_unused]] = void;                          \
        } val;                                                                 \
        return val;                                                            \
    }()

namespace {
using namespace msg;

using opcode_field =
    field<"opcode_field", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;

using sub_opcode_field =
    msg::field<"sub_opcode_field",
               std::uint32_t>::located<at{0_dw, 15_msb, 0_lsb}>;

using field_3 =
    msg::field<"field_3", std::uint32_t>::located<at{1_dw, 23_msb, 16_lsb}>;

using field_4 =
    msg::field<"field_5", std::uint32_t>::located<at{1_dw, 15_msb, 0_lsb}>;

using msg_defn =
    message<"test_msg", opcode_field, sub_opcode_field, field_3, field_4>;
using test_msg = owning<msg_defn>;

template <auto N> using bitset = stdx::bitset<N, std::uint32_t>;

bitset<32> callbacks_called{};
} // namespace

TEST_CASE("create empty handler", "[indexed_handler]") {
    [[maybe_unused]] constexpr auto h = msg::indexed_handler{
        msg::callback_args<test_msg>,
        msg::indices{msg::index{
            opcode_field{}, lookup::make(CX_VALUE(
                                lookup::input<std::uint32_t, bitset<32>>{}))}},
        std::array<void (*)(test_msg const &), 0>{}};
}

TEST_CASE("create handler with one index and callback", "[indexed_handler]") {
    constexpr auto h = msg::indexed_handler{
        msg::callback_args<test_msg>,
        msg::indices{msg::index{
            opcode_field{},
            lookup::make(CX_VALUE(lookup::input{
                bitset<32>{}, std::array{lookup::entry{
                                  42u, bitset<32>{stdx::place_bits, 0}}}}))}},
        std::array<void (*)(test_msg const &), 1>{
            [](test_msg const &) { callbacks_called.set(0); }}};

    callbacks_called.reset();
    h.handle(test_msg{"opcode_field"_field = 42});
    CHECK(h.is_match(test_msg{"opcode_field"_field = 42}));
    CHECK(callbacks_called[0]);

    callbacks_called.reset();
    h.handle(test_msg{"opcode_field"_field = 12});
    CHECK(not h.is_match(test_msg{"opcode_field"_field = 12}));
    CHECK(callbacks_called.none());
}

TEST_CASE("create handler with multiple indices and callbacks",
          "[indexed_handler]") {
    using lookup::entry;

    constexpr auto h = msg::indexed_handler{
        msg::callback_args<test_msg>,
        msg::indices{
            msg::index{
                opcode_field{},
                lookup::make(CX_VALUE(lookup::input{
                    bitset<32>{},
                    std::array{
                        entry{0u, bitset<32>{stdx::place_bits, 0, 1, 2, 3}},
                        entry{1u, bitset<32>{stdx::place_bits, 4, 5, 6, 7}},
                        entry{2u, bitset<32>{stdx::place_bits, 8}}}}))},
            msg::index{sub_opcode_field{},
                       lookup::make(CX_VALUE(lookup::input{
                           bitset<32>{stdx::place_bits, 8},
                           std::array{
                               entry{0u, bitset<32>{stdx::place_bits, 0, 4, 8}},
                               entry{1u, bitset<32>{stdx::place_bits, 1, 5, 8}},
                               entry{2u, bitset<32>{stdx::place_bits, 2, 6, 8}},
                               entry{3u, bitset<32>{stdx::place_bits, 3, 7, 8}},
                           }}))}},
        std::array<void (*)(test_msg const &), 9>{
            [](test_msg const &) { callbacks_called.set(0); },
            [](test_msg const &) { callbacks_called.set(1); },
            [](test_msg const &) { callbacks_called.set(2); },
            [](test_msg const &) { callbacks_called.set(3); },
            [](test_msg const &) { callbacks_called.set(4); },
            [](test_msg const &) { callbacks_called.set(5); },
            [](test_msg const &) { callbacks_called.set(6); },
            [](test_msg const &) { callbacks_called.set(7); },
            [](test_msg const &) { callbacks_called.set(8); }}};

    auto const check_msg = [&](std::uint32_t op, std::uint32_t sub_op,
                               std::size_t callback_index) {
        callbacks_called.reset();
        h.handle(test_msg{"opcode_field"_field = op,
                          "sub_opcode_field"_field = sub_op});
        CHECK(callbacks_called[callback_index]);
    };

    check_msg(0, 0, 0);
    check_msg(0, 1, 1);
    check_msg(0, 2, 2);
    check_msg(0, 3, 3);
    check_msg(1, 0, 4);
    check_msg(1, 1, 5);
    check_msg(1, 2, 6);
    check_msg(1, 3, 7);

    check_msg(2, 0, 8);
    check_msg(2, 1, 8);
    check_msg(2, 2, 8);
    check_msg(2, 3, 8);
    check_msg(2, 42, 8);

    auto const check_no_match = [&](std::uint32_t op, std::uint32_t sub_op) {
        callbacks_called.reset();
        h.handle(test_msg{"opcode_field"_field = op,
                          "sub_opcode_field"_field = sub_op});
        CHECK(callbacks_called.none());
    };

    check_no_match(3, 0);
    check_no_match(0, 4);
    check_no_match(1, 4);
}

#undef CX_VALUE
