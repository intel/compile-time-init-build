#include <msg/indexed_handler.hpp>
#include <msg/detail/bitset.hpp>
#include <lookup/lookup.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>

namespace msg {

using opcode_field = field<decltype("opcode_field"_sc), 0, 31, 24, std::uint32_t>;

using sub_opcode_field = field<decltype("sub_opcode_field"_sc), 0, 15, 0, std::uint32_t>;

using field_3 = field<decltype("field_3"_sc), 1, 23, 16, std::uint32_t>;

using field_4 = field<decltype("field_4"_sc), 1, 15, 0, std::uint32_t>;

using test_msg =
    message_base<decltype("test_msg"_sc), 4, 2, opcode_field, sub_opcode_field, field_3, field_4>;

TEST_CASE("create empty handler", "[indexed_handler]") {
    using lookup::input;
    using msg::detail::bitset;

    [[maybe_unused]] constexpr auto h = indexed_handler{
        callback_args<test_msg>,
        indices{
            index{opcode_field{}, lookup::make<input<uint32_t, bitset<32>, bitset<32>{}>>()}
        },
        std::array<void(*)(test_msg const &), 0>{}
    };
}

msg::detail::bitset<32> callbacks_called{};

TEST_CASE("create handler with one index and callback", "[indexed_handler]") {
    using lookup::input;
    using lookup::entry;
    using msg::detail::bitset;

    constexpr auto h = indexed_handler{
        callback_args<test_msg>,

        indices{
            index{opcode_field{}, lookup::make<input<uint32_t, bitset<32>,
                bitset<32>{},
                entry{42u, bitset<32>{0}}
            >>()}
        },

        std::array<void(*)(test_msg const &), 1>{
            [](test_msg const &){
                callbacks_called.add(0);
            }
        }
    };

    callbacks_called = {};
    h.handle(test_msg{opcode_field{42}});
    REQUIRE(h.is_match(test_msg{opcode_field{42}}));
    REQUIRE(callbacks_called == bitset<32>{0});

    callbacks_called = {};
    h.handle(test_msg{opcode_field{12}});
    REQUIRE_FALSE(h.is_match(test_msg{opcode_field{12}}));
    REQUIRE(callbacks_called == bitset<32>{});
}

TEST_CASE("create handler with multiple indices and callbacks", "[indexed_handler]") {
    using lookup::input;
    using lookup::entry;
    using msg::detail::bitset;

    constexpr auto h = indexed_handler{
        callback_args<test_msg>,

        indices{
            index{opcode_field{}, lookup::make<input<uint32_t, bitset<32>,
                bitset<32>{},
                entry{0u, bitset<32>{0, 1, 2, 3}},
                entry{1u, bitset<32>{4, 5, 6, 7}},
                entry{2u, bitset<32>{8}}
            >>()},

            index{sub_opcode_field{}, lookup::make<input<uint32_t, bitset<32>,
                bitset<32>{8},
                entry{0u, bitset<32>{0, 4, 8}},
                entry{1u, bitset<32>{1, 5, 8}},
                entry{2u, bitset<32>{2, 6, 8}},
                entry{3u, bitset<32>{3, 7, 8}}
            >>()}
        },

        std::array<void(*)(test_msg const &), 9>{
            [](test_msg const &){callbacks_called.add(0);},
            [](test_msg const &){callbacks_called.add(1);},
            [](test_msg const &){callbacks_called.add(2);},
            [](test_msg const &){callbacks_called.add(3);},
            [](test_msg const &){callbacks_called.add(4);},
            [](test_msg const &){callbacks_called.add(5);},
            [](test_msg const &){callbacks_called.add(6);},
            [](test_msg const &){callbacks_called.add(7);},
            [](test_msg const &){callbacks_called.add(8);}
        }
    };

    auto const check_msg = [&](std::uint32_t op, std::uint32_t sub_op, std::size_t callback_index){
        callbacks_called = {};
        h.handle(test_msg{opcode_field{op}, sub_opcode_field{sub_op}});
        REQUIRE(callbacks_called == bitset<32>{callback_index});
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

    auto const check_no_match = [&](std::uint32_t op, std::uint32_t sub_op){
        callbacks_called = {};
        h.handle(test_msg{opcode_field{op}, sub_opcode_field{sub_op}});
        REQUIRE(callbacks_called == bitset<32>{});
    };

    check_no_match(3, 0);
    check_no_match(0, 4);
    check_no_match(1, 4);
}
} // namespace msg
