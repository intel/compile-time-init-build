#pragma once

#include <cstdint>

namespace ns {
enum struct E1 { VAL_E1 = 19 };
enum E2 { VAL_E2 = 23 };

enum struct E_bool : bool { VALUE = true };
enum struct E_8bit : std::uint8_t { VALUE = 0x12U };
enum struct E_16bit : std::uint16_t { VALUE = 0x1234U };
enum struct E_32bit : std::uint32_t { VALUE = 0x1234'5678U };
enum struct E_64bit : std::uint64_t { VALUE = 0x1234'5678'90ab'cdefULL };
} // namespace ns
