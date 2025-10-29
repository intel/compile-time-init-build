#include <msg/message.hpp>

// EXPECT: Old field name not found in message
namespace {
using namespace msg;

using test_field1 =
    field<"f1", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;

using msg_defn = message<"test_msg", test_field1>;
} // namespace

auto main() -> int { using M = rename_field<msg_defn, "f", "F">; }
