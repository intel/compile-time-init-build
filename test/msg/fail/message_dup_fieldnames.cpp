#include <msg/field.hpp>
#include <msg/message.hpp>

// EXPECT: Message contains fields with duplicate names
namespace {
using namespace msg;

using test_field1 =
    field<"test_field", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;
using test_field2 =
    field<"test_field", std::uint32_t>::located<at{0_dw, 7_msb, 0_lsb}>;

using msg_defn = message<"test_msg", test_field1, test_field2>;
} // namespace

auto main() -> int { [[maybe_unused]] msg_defn m{}; }
