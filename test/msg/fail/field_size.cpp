#include <msg/field.hpp>

#include <cstdint>

// EXPECT: Field size is smaller than sum of locations
using namespace msg;
using F =
    field<"", std::uint8_t>::located<at{3_msb, 0_lsb}, at{14_msb, 10_lsb}>;
F f{};
