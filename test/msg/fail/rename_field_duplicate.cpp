#include <msg/message.hpp>

// EXPECT: New field name already exists in message
namespace {
using namespace msg;

using test_field1 =
    field<"f1", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;
using test_field2 =
    field<"f2", std::uint32_t>::located<at{0_dw, 23_msb, 16_lsb}>;

using msg_defn = message<"test_msg", test_field1, test_field2>;
} // namespace

auto main() -> int { using M = rename_field<msg_defn, "f1", "f2">; }
