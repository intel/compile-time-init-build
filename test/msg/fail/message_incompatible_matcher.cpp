#include <msg/field.hpp>
#include <msg/message.hpp>

// EXPECT: (constraints not satisfied for)|(template constraint failure for)
namespace {
using namespace msg;

using f = field<"f", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;

using msg_defn =
    message<"test_msg", f::with_const_default<1>::with_equal_to<2>>;
} // namespace

auto main() -> int {}
