#include <msg/field.hpp>
#include <msg/message.hpp>

// EXPECT: Field does not belong to this message
namespace {
using namespace msg;

using test_field1 =
    field<"test_field", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;

using msg_defn = message<"test_msg", test_field1>;
} // namespace

auto main() -> int {
    owning<msg_defn> m{};
    m.set("no"_field = 1);
}
