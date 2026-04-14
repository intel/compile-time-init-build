#include <interrupt/config.hpp>
#include <interrupt/dynamic_controller.hpp>

// EXPECT: disable flow \(void\) not in config!

using interrupt::operator""_irq;

using config_t = interrupt::root<
    interrupt::irq<"", 0_irq, 0, interrupt::no_field_t, interrupt::policies<>>>;

struct test_hal {
    static auto init() -> void {}
    template <auto...> static auto irq_init() -> void {}

    template <typename Field> CONSTEVAL static auto get_field() -> Field {
        return Field{};
    }
    template <typename Field> CONSTEVAL static auto get_register() -> Field {
        return Field{};
    }

    template <typename Register> using register_datatype_t = std::uint32_t;

    template <typename Register, typename Field>
    constexpr static register_datatype_t<Register> mask = {};

    static auto write(auto, auto) -> void {}
    static auto read(auto) -> bool { return true; }
    static auto clear(auto) -> void {}
};

auto main() -> int {
    using dynamic_t = interrupt::dynamic_controller<config_t, test_hal>;
    dynamic_t::disable<void>();
}
