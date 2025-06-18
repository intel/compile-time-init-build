#include <msg/field.hpp>

#include <cstdint>

// EXPECT: Field location is outside the range of argument

using namespace msg;
using F = field<"", std::uint8_t>::located<at{32_msb, 31_lsb}>;

auto main() -> int {
    std::uint32_t data{};
    F::insert(data, 1u);
}
