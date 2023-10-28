#include <msg/field.hpp>

#include <cstdint>

// EXPECT: Individual field location size cannot exceed 64 bits
using namespace msg;
using F = field<"", std::uint64_t>::located<at{0_dw, 64_msb, 0_lsb}>;
F f{};
